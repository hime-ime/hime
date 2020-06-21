/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation version 2.1
 * of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pwd.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "hime.h"

#include "hime-im-client.h"
#include "hime-protocol.h"
#include "im-srv.h"

#define DBG 0

HIME_ENT *hime_clients;
int hime_clientsN;

static gboolean is_init_im_enabled = FALSE;

// from eve.c
extern char *output_buffer;
extern uint32_t output_bufferN;

static ssize_t read_enc (const int fd, void *p, const size_t n) {
    if (!fd) {
        return 0;
    }

    if (!p || n == 0) {
        return 0;
    }

    const ssize_t r = read (fd, p, n);
    if (r <= 0) {
        return r;
    }

    if (hime_clients[fd].type == Connection_type_tcp) {
        __hime_enc_mem ((unsigned char *) p,
                        n,
                        &srv_ip_port.passwd,
                        &hime_clients[fd].seed);
    }

    return r;
}

static ssize_t write_enc (const int fd, const void *p, const size_t n) {
    if (!fd) {
        return 0;
    }

    if (!p || n == 0) {
        return 0;
    }

    unsigned char *tmp = malloc (n);
    if (!tmp) {
        perror ("write_enc: malloc failed");
        return 0;
    }

    memcpy (tmp, p, n);

    if (hime_clients[fd].type == Connection_type_tcp) {
        __hime_enc_mem (tmp, n, &srv_ip_port.passwd, &hime_clients[fd].seed);
    }

    const ssize_t r = write (fd, tmp, n);
    if (r < 0) {
        perror ("write_enc: failed to write into fd");
    }

    free (tmp);

    return r;
}

static void shutdown_client (const int fd) {
    const int idx = fd;

    g_source_remove (hime_clients[idx].tag);

    if (hime_clients[idx].cs == current_CS) {
        hide_in_win (current_CS);
        current_CS = NULL;
    }

    free (hime_clients[idx].cs);
    hime_clients[idx].cs = NULL;
    hime_clients[idx].fd = 0;

    close (fd);
}

static void parse_client_req (HIME_req *req) {
    to_hime_endian_4 (&req->req_no);
    to_hime_endian_4 (&req->client_win);
    to_hime_endian_4 (&req->flag);
    to_hime_endian_2 (&req->spot_location.x);
    to_hime_endian_2 (&req->spot_location.y);
}

static void write_reply (HIME_reply *reply, const int fd) {

    const uint32_t datalen = reply->datalen =
        output_bufferN ? output_bufferN + 1 : 0;  // include '\0'

    to_hime_endian_4 (&reply->flag);
    to_hime_endian_4 (&reply->datalen);

    write_enc (fd, reply, sizeof (*reply));

    if (output_bufferN) {
        write_enc (fd, output_buffer, datalen);
        clear_output_buffer ();
    }
}

static void do_process_key (HIME_req *req,
                            HIME_reply *reply,
                            const int fd,
                            ClientState *cs) {

    current_CS = cs;
    save_CS_temp_to_current ();

    to_hime_endian_4 (&req->key_event.key);
    to_hime_endian_4 (&req->key_event.state);

    char *typed = NULL;
    gboolean status = FALSE;

    if (req->req_no == HIME_req_key_press) {
        status = ProcessKeyPress (req->key_event.key, req->key_event.state);
        typed = "press";
    } else {
        status = ProcessKeyRelease (req->key_event.key, req->key_event.state);
        typed = "release";
    }

    if (status) {
        reply->flag |= HIME_reply_key_processed;
    }

    dbg ("%s srv flag:%x status:%d len:%d %x %c\n",
         typed,
         reply->flag,
         status,
         output_bufferN,
         req->key_event.key,
         req->key_event.key & 0x7f);

    write_reply (reply, fd);
}

static void do_set_flags (HIME_req *req,
                          const int fd,
                          ClientState *cs) {

    if (BITON (req->flag, FLAG_HIME_client_handle_raise_window)) {
        dbg ("********* raise * window\n");
        if (!hime_pop_up_win) {
            cs->b_raise_window = TRUE;
        }
    }

    if (req->flag & FLAG_HIME_client_handle_use_preedit) {
        cs->use_preedit = TRUE;
    }

    int rflags = 0;
    if (hime_pop_up_win) {
        rflags = FLAG_HIME_srv_ret_status_use_pop_up;
    }

    write_enc (fd, &rflags, sizeof (rflags));
}

static void do_get_preedit (const int fd, ClientState *cs) {

    dbg ("svr HIME_req_get_preedit %x\n", cs);

    char str[HIME_PREEDIT_MAX_STR];
    HIME_PREEDIT_ATTR attr[HIME_PREEDIT_ATTR_MAX_N];
    int cursor = 0;
    int sub_comp_len = 0;

    int attrN = hime_get_preedit (cs, str, attr, &cursor, &sub_comp_len);

    if (hime_edit_display & (HIME_EDIT_DISPLAY_BOTH | HIME_EDIT_DISPLAY_OVER_THE_SPOT)) {
        cursor = 0;
    }

    if (hime_edit_display & HIME_EDIT_DISPLAY_OVER_THE_SPOT) {
        attrN = 0;
        str[0] = '\0';
    }

    // XXX(xatier): should use size_t
    const int len = strlen (str) + 1;  // including \0

    write_enc (fd, &len, sizeof (len));
    write_enc (fd, str, len);
    write_enc (fd, &attrN, sizeof (attrN));

    if (attrN > 0) {
        write_enc (fd, attr, sizeof (HIME_PREEDIT_ATTR) * attrN);
    }

    write_enc (fd, &cursor, sizeof (cursor));
    write_enc (fd, &sub_comp_len, sizeof (sub_comp_len));
}

static void do_req_message (const int fd) {

    // XXX(xatier): should use size_t
    short len = 0;
    if (read (fd, &len, sizeof (len)) <= 0) {
        shutdown_client (fd);
        return;
    }

    // only unix socket, no decrypt
    // message should include '\0'
    char buf[512];
    if (len > 0 && len < sizeof (buf)) {
        if (read (fd, buf, len) <= 0) {
            shutdown_client (fd);
            return;
        }
        message_cb (buf);
    }
}

static void do_invalid_req (const int fd) {
    struct sockaddr_in addr;
    memset (&addr, 0, sizeof (addr));

    socklen_t len = sizeof (addr);
    if (!getpeername (fd, (struct sockaddr *) &addr, &len)) {
        dbg ("%s\n", inet_ntoa (addr.sin_addr));
    } else {
        perror ("getpeername\n");
    }

    shutdown_client (fd);
}

void process_client_req (const int fd) {

    dbg ("svr--> process_client_req %d\n", fd);

    HIME_req req;
    const ssize_t r = read_enc (fd, &req, sizeof (req));
    if (r <= 0) {
        shutdown_client (fd);
        return;
    }

    parse_client_req (&req);

    ClientState *cs = NULL;
    if (current_CS && (req.client_win == current_CS->client_win)) {
        cs = current_CS;
    } else {
        cs = hime_clients[fd].cs;

        int new_cli = 0;
        if (!cs) {
            cs = hime_clients[fd].cs = tzmalloc (ClientState, 1);
            new_cli = 1;
        }

        cs->client_win = req.client_win;
        cs->b_hime_protocol = TRUE;
        cs->input_style = InputStyleOverSpot;

        if (hime_init_im_enabled) {
            if (
                (hime_single_state && !is_init_im_enabled) ||
                (!hime_single_state && new_cli)) {

                dbg ("new_cli default_input_method:%d\n", default_input_method);

                is_init_im_enabled = TRUE;
                current_CS = cs;
                save_CS_temp_to_current ();
                init_state_chinese (cs);
            }
        }
    }

    if (!cs) {
        p_err ("bad cs\n");
    }

    if (req.req_no != HIME_req_message) {
        cs->spot_location.x = req.spot_location.x;
        cs->spot_location.y = req.spot_location.y;
    }

    HIME_reply reply;
    memset (&reply, 0, sizeof (reply));

    switch (req.req_no) {
    case HIME_req_key_press:
    case HIME_req_key_release:
        do_process_key (&req, &reply, fd, cs);
        break;

    case HIME_req_focus_in:
        dbg_time ("HIME_req_focus_in  %x %d %d\n", cs, cs->spot_location.x, cs->spot_location.y);
        hime_FocusIn (cs);
        break;

    case HIME_req_focus_out:
        dbg_time ("HIME_req_focus_out  %x\n", cs);
        hime_FocusOut (cs);
        break;

    case HIME_req_focus_out2:
        dbg_time ("HIME_req_focus_out2  %x\n", cs);
        if (hime_FocusOut (cs)) {
            flush_edit_buffer ();
        }

        write_reply (&reply, fd);
        break;

    case HIME_req_set_cursor_location:
        dbg_time ("set_cursor_location %x %d %d\n", cs,
                  cs->spot_location.x, cs->spot_location.y);
        update_in_win_pos ();
        break;

    case HIME_req_set_flags:
        do_set_flags (&req, fd, cs);
        break;

    case HIME_req_get_preedit:
        do_get_preedit (fd, cs);
        break;

    case HIME_req_reset:
        hime_reset ();
        break;

    case HIME_req_message:
        do_req_message (fd);
        break;

    default:
        dbg_time ("Invalid request %x from:", req.req_no);
        do_invalid_req (fd);
        break;
    }
}
