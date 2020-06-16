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

#include <errno.h>
#include <signal.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <X11/Xlib.h>

#include "hime.h"

#include "hime-im-client.h"
#include "hime-protocol.h"
#include "im-srv.h"

#define DBG 0

// will be restored in hime_im_client_reopen
static int flags_backup;

static int __is_special_user;
static void init_is_special_user (void) {
    // memoize getuid()
    const int uid = getuid ();
    __is_special_user = 0 < uid && uid < 500;
}

static int is_special_user (void) {
    return __is_special_user;
}

static int skip_processing (const HIME_client_handle *handle) {
    return !handle || is_special_user ();
}

static unsigned char *get_window_property (Display *display,
                                           Window hime_win,
                                           Atom atom) {
    Atom actual_type_return;
    int actual_format_return = 0;
    unsigned long nitems_return = 0;
    unsigned long bytes_after_return = 0;
    unsigned char *prop_return = NULL;

    int success = XGetWindowProperty (display,
                                      hime_win,
                                      atom,
                                      0, 64, /* offset / length */
                                      False, /* delete */
                                      AnyPropertyType,
                                      &actual_type_return,
                                      &actual_format_return,
                                      &nitems_return,
                                      &bytes_after_return,
                                      &prop_return);

    if (success != Success) {
        return NULL;
    }

    return prop_return;
}

static unsigned char *get_sockpath_atom (Display *display, Window hime_win) {

    Atom hime_sockpath_atom = get_hime_sockpath_atom (display);

    if (!hime_sockpath_atom) {
        return NULL;
    }

    return get_window_property (display, hime_win, hime_sockpath_atom);
}

static unsigned char *get_addr_atom (Display *display, Window hime_win) {

    Atom hime_addr_atom = get_hime_addr_atom (display);

    if (!hime_addr_atom) {
        return NULL;
    }

    return get_window_property (display, hime_win, hime_addr_atom);
}

static void init_unix_serv_addr (const unsigned char *message_sock,
                                 struct sockaddr_un *serv_addr) {

    memset (serv_addr, 0, sizeof (*serv_addr));

    serv_addr->sun_family = AF_UNIX;

    Server_sock_path srv_sock_path;
    srv_sock_path.sock_path[0] = '\0';
    memcpy (&srv_sock_path, message_sock, sizeof (srv_sock_path));

    char sock_path[UNIX_PATH_MAX];
    if (srv_sock_path.sock_path[0]) {
        strncpy (sock_path, srv_sock_path.sock_path, UNIX_PATH_MAX);
    } else {
        get_hime_im_srv_sock_path (sock_path, sizeof (sock_path));
    }

    strcpy (serv_addr->sun_path, sock_path);
}

static void init_ipv4_serv_addr (const Server_IP_port *srv_ip_port,
                                 struct sockaddr_in *serv_addr) {

    memset (serv_addr, 0, sizeof (*serv_addr));

    serv_addr->sin_family = AF_INET;

    serv_addr->sin_addr.s_addr = srv_ip_port->ip;
    serv_addr->sin_port = srv_ip_port->port;
}

static HIME_client_handle *hime_im_client_reopen (HIME_client_handle *hime_ch,
                                                  Display *display) {

    const int dbg_msg = getenv ("HIME_CONNECT_MSG_ON") != NULL;

    init_is_special_user ();

    if (!display) {
        dbg ("display is null fd: %d\n", hime_ch->fd);
        goto next;
    }

    Window hime_win = None;

    const int MAX_TRY = 3;
    int loop = 0;

    // obtain hime_win and fork
    if (!is_special_user ()) {
        for (loop = 0; loop < MAX_TRY; loop++) {
            if ((hime_win = find_hime_window (display)) != None ||
                getenv ("HIME_IM_CLIENT_NO_AUTO_EXEC")) {
                break;
            }
            static time_t exec_time;

            if (time (NULL) - exec_time > 1) {
                time (&exec_time);

                dbg ("XGetSelectionOwner: old version of hime or hime is not running ?\n");

                static char execbin[] = HIME_BIN_DIR "/hime";
                dbg ("... try to start a new hime server %s\n", execbin);

                int pid;

                if ((pid = fork ()) == 0) {
                    setenv ("HIME_DAEMON", "", TRUE);
                    execl (execbin, "hime", NULL);
                } else {
                    int status;
                    // hime will daemon()
                    waitpid (pid, &status, 0);
                }
            }
        }
    }

    if (loop == MAX_TRY || hime_win == None) {
        goto next;
    }

    // -----------------------------------------------------------------------
    // The below logic tries to prepare a valid sockfd.
    //
    // we try to create UNIX domain socket first,
    // (if failed,) we try to create a IPv4 socket (ipv4 flag will be set).
    // -----------------------------------------------------------------------

    int sockfd = 0;
    int ipv4 = FALSE;

    // -----------------------------------------------------------------------
    // trying to create a UNIX domain socket (AF_UNIX) connection
    // -----------------------------------------------------------------------

    // get HIME socket path from X window (HIME_SOCKPATH_ATOM)
    unsigned char *unix_message_sock = get_sockpath_atom (display, hime_win);
    if (!unix_message_sock) {
        dbg ("[UNIX] XGetWindowProperty: old version of hime or hime is not running ?\n");
        goto next;
    }

    // UNIX domain socket (AF_UNIX)
    struct sockaddr_un serv_addr;
    init_unix_serv_addr (unix_message_sock, &serv_addr);
    XFree (unix_message_sock);
    unix_message_sock = NULL;

    if ((sockfd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror ("cannot open UNIX domain socket");
        goto tcp;
    }

    if (connect (sockfd, (struct sockaddr *) &serv_addr, SUN_LEN (&serv_addr)) < 0) {
        perror ("cannot connect to UNIX domain socket");
        close (sockfd);
        sockfd = 0;
        goto tcp;
    }

    if (dbg_msg) {
        dbg ("connected to unix socket addr %s\n", serv_addr.sun_path);
    }

    // we are now connected to a UNIX domain socket
    goto next;

tcp:;
    // -----------------------------------------------------------------------
    // trying to create a IPv4 socket (AF_INET) connection
    // -----------------------------------------------------------------------
    //
    // get HIME address from X window (HIME_ADDR_ATOM)
    unsigned char *ipv4_message_sock = get_addr_atom (display, hime_win);
    if (!ipv4_message_sock) {
        dbg ("[IPv4] XGetWindowProperty: old version of hime or hime is not running ?\n");
        goto next;
    }

    Server_IP_port srv_ip_port;
    memcpy (&srv_ip_port, ipv4_message_sock, sizeof (srv_ip_port));
    XFree (ipv4_message_sock);
    ipv4_message_sock = NULL;

    // IPv4 socket (AF_INET)
    struct sockaddr_in in_serv_addr;
    init_ipv4_serv_addr (&srv_ip_port, &in_serv_addr);

    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("cannot open IPv4 socket");
        goto next;
    }

    if (connect (sockfd, (struct sockaddr *) &in_serv_addr, sizeof (in_serv_addr)) < 0) {
        perror ("cannot cannot connect to IPv4 socket");
        close (sockfd);
        sockfd = 0;
        goto next;
    }

    // debug message
    if (dbg_msg) {
        unsigned char *pp = (unsigned char *) &srv_ip_port.ip;
        dbg ("hime client connected to server %d.%d.%d.%d:%d\n",
             pp[0], pp[1], pp[2], pp[3], ntohs (srv_ip_port.port));
    }

    // we are now connected to a IPv4 socket
    ipv4 = TRUE;
    goto next;

next:;
    HIME_client_handle *handle = hime_ch ? hime_ch : tzmalloc (HIME_client_handle, 1);

    if (sockfd < 0) {
        sockfd = 0;
    }

    if (sockfd > 0) {
        handle->fd = sockfd;
        if (ipv4) {
            if (!handle->passwd) {
                handle->passwd = malloc (sizeof (HIME_PASSWD));
            }
            memcpy (handle->passwd, &srv_ip_port.passwd, sizeof (srv_ip_port.passwd));
        } else {
            if (handle->passwd) {
                free (handle->passwd);
                handle->passwd = NULL;
            }
        }
    }

    if (handle->fd) {
        if (BITON (handle->flag, FLAG_HIME_client_handle_has_focus)) {
            hime_im_client_focus_in (handle);
        }

        int rstatus = 0;
        hime_im_client_set_flags (handle, flags_backup, &rstatus);
    }

    return handle;
}

static void validate_handle (HIME_client_handle *handle) {
    if (handle->fd > 0) {
        return;
    }

    if (is_special_user ()) {
        return;
    }

    hime_im_client_reopen (handle, handle->display);
}

static int gen_req (HIME_client_handle *handle,
                    const HIME_req_t req_no,
                    HIME_req *req) {

    validate_handle (handle);

    if (!handle || !handle->fd) {
        return 0;
    }

    handle->seq++;

    memset (req, 0, sizeof (HIME_req));

    req->req_no = req_no;
    to_hime_endian_4 (&req->req_no);

    req->client_win = handle->client_win;
    to_hime_endian_4 (&req->client_win);

    req->flag = 0;
    to_hime_endian_4 (&req->flag);

    req->input_style = handle->input_style;
    to_hime_endian_4 (&req->input_style);

    req->spot_location.x = handle->spot_location.x;
    req->spot_location.y = handle->spot_location.y;
    to_hime_endian_2 (&req->spot_location.x);
    to_hime_endian_2 (&req->spot_location.y);

    return 1;
}

static void error_proc (HIME_client_handle *handle, const char *msg) {
    if (!handle || !handle->fd) {
        return;
    }

    perror (msg);
    close (handle->fd);
    handle->fd = 0;
    usleep (100000);
}

typedef struct {
    struct sigaction pipe;
} SAVE_ACT;

static void save_old_sigaction (SAVE_ACT *save_act) {
    const int signo = SIGPIPE;
    struct sigaction *act = &save_act->pipe;

    sigaction (signo, NULL, act);

    if (act->sa_handler != SIG_IGN) {
        signal (signo, SIG_IGN);
    }
}

static void restore_old_sigaction (SAVE_ACT *save_act) {
    const int signo = SIGPIPE;
    struct sigaction *act = &save_act->pipe;

    if (act->sa_handler != SIG_IGN) {
        signal (signo, act->sa_handler);
    }
}

// read from hime server
static ssize_t handle_read (HIME_client_handle *handle,
                            void *ptr,
                            const int n) {

    if (!handle || !handle->fd) {
        return 0;
    }

    SAVE_ACT save_act;
    save_old_sigaction (&save_act);

    const ssize_t r = read (handle->fd, ptr, n);
    if (r < 0) {
        perror ("handle_read");
    }

    restore_old_sigaction (&save_act);

    if (r <= 0) {
        return r;
    }

    if (handle->passwd) {
        __hime_enc_mem ((unsigned char *) ptr, n, handle->passwd, &handle->passwd->seed);
    }

    return r;
}

// write to hime server
static ssize_t handle_write (HIME_client_handle *handle,
                             const void *ptr,
                             const int n) {

    if (!handle || !handle->fd) {
        return 0;
    }

    unsigned char *tmp = malloc (n);
    if (!tmp) {
        return 0;
    }
    memcpy (tmp, ptr, n);

    if (handle->passwd) {
        __hime_enc_mem (tmp, n, handle->passwd, &handle->passwd->seed);
    }

    SAVE_ACT save_act;
    save_old_sigaction (&save_act);

    const ssize_t r = write (handle->fd, tmp, n);
    if (r == -1) {
        perror ("handle_write");
    }

    restore_old_sigaction (&save_act);

    if (r <= 0) {
        return r;
    }

    free (tmp);
    tmp = NULL;

    return r;
}

// public functions
// APIs for Gtk+/Qt IM modules
HIME_client_handle *hime_im_client_open (Display *display) {
    HIME_client_handle *handle = hime_im_client_reopen (NULL, display);
    handle->display = display;
    return handle;
}

void hime_im_client_close (HIME_client_handle *handle) {
    if (!handle) {
        return;
    }

    if (handle->fd > 0) {
        close (handle->fd);
    }

    free (handle->passwd);
    handle->passwd = NULL;
    free (handle);
    handle = NULL;
}

void hime_im_client_set_client_window (HIME_client_handle *handle,
                                       const Window win) {
    if (skip_processing (handle)) {
        return;
    }

    if (win == None) {
        return;
    }

    handle->client_win = win;
}

int hime_im_client_get_preedit (HIME_client_handle *handle,
                                char **str,
                                HIME_PREEDIT_ATTR attr[],
                                int *cursor,
                                int *sub_comp_len) {
    // output string
    *str = NULL;

    if (skip_processing (handle)) {
        return 0;
    }

    HIME_req req;
    if (!gen_req (handle, HIME_req_get_preedit, &req)) {
    err_ret:
        // error case, return an empty string with cursor position = 0
        if (cursor) {
            *cursor = 0;
        }
        *str = strdup ("");
        return 0;
    }

    if (handle_write (handle, &req, sizeof (req)) <= 0) {
        error_proc (handle, "hime_im_client_get_preedit error, cannot write req");
        goto err_ret;
    }

    int str_len = 0;
    if (handle_read (handle, &str_len, sizeof (str_len)) <= 0) {
        goto err_ret;
    }

    *str = (char *) malloc (str_len);
    if (!(*str)) {
        goto err_ret;
    }

    if (handle_read (handle, *str, str_len) <= 0) {
        free (*str);
        *str = NULL;
        goto err_ret;
    }

#if DBG
    dbg ("hime_im_client_get_preedit len:%d '%s' \n", str_len, *str);
#endif

    int attrN = 0;
    if (handle_read (handle, &attrN, sizeof (attrN)) <= 0) {
        goto err_ret;
    }

    if (attrN > 0 && handle_read (handle, attr, sizeof (HIME_PREEDIT_ATTR) * attrN) <= 0) {
        goto err_ret;
    }

    int tcursor = 0;
    if (handle_read (handle, &tcursor, sizeof (tcursor)) <= 0) {
        goto err_ret;
    }

    if (cursor) {
        *cursor = tcursor;
    }

    int tsub_comp_len = 0;
    if (handle_read (handle, &tsub_comp_len, sizeof (tsub_comp_len)) <= 0) {
        goto err_ret;
    }

    if (sub_comp_len) {
        *sub_comp_len = tsub_comp_len;
    }

    return attrN;
}

static int hime_im_client_forward_key_event (HIME_client_handle *handle,
                                             const HIME_req_t event_type,
                                             const KeySym key,
                                             const uint32_t state,
                                             char **rstr) {

    if (rstr) {
        *rstr = NULL;
    }

    if (skip_processing (handle)) {
        return FALSE;
    }

    HIME_req req;
    if (!gen_req (handle, event_type, &req)) {
        return FALSE;
    }

    req.key_event.key = key;
    req.key_event.state = state;

    to_hime_endian_4 (&req.key_event.key);
    to_hime_endian_4 (&req.key_event.state);

    if (handle_write (handle, &req, sizeof (req)) <= 0) {
        error_proc (handle, "cannot write to hime server");
        return FALSE;
    }

    HIME_reply reply;
    memset (&reply, 0, sizeof (reply));
    if (handle_read (handle, &reply, sizeof (reply)) <= 0) {
        error_proc (handle, "cannot read reply from hime server");
        return FALSE;
    }

    to_hime_endian_4 (&reply.datalen);
    to_hime_endian_4 (&reply.flag);

    if (reply.datalen > 0) {
        *rstr = (char *) malloc (reply.datalen);
        if (handle_read (handle, *rstr, reply.datalen) <= 0) {
            free (*rstr);
            *rstr = NULL;
            error_proc (handle, "cannot read reply str from hime server");
            return FALSE;
        }
    }

    return reply.flag;
}

// return TRUE if the key is accepted
int hime_im_client_forward_key_press (HIME_client_handle *handle,
                                      const KeySym key,
                                      const uint32_t state,
                                      char **rstr) {
    if (!handle) {
        return FALSE;
    }

    // in case client didn't send focus in event
    if (!BITON (handle->flag, FLAG_HIME_client_handle_has_focus)) {
        hime_im_client_focus_in (handle);
        handle->flag |= FLAG_HIME_client_handle_has_focus;
        hime_im_client_set_cursor_location (handle, handle->spot_location.x,
                                            handle->spot_location.y);
    }

    int flag = hime_im_client_forward_key_event (
        handle, HIME_req_key_press, key, state, rstr);

    return ((flag & HIME_reply_key_processed) != 0);
}

// return TRUE if the key is accepted
int hime_im_client_forward_key_release (HIME_client_handle *handle,
                                        const KeySym key,
                                        const uint32_t state,
                                        char **rstr) {
    if (!handle) {
        return FALSE;
    }

    handle->flag |= FLAG_HIME_client_handle_has_focus;

    int flag = hime_im_client_forward_key_event (
        handle, HIME_req_key_release, key, state, rstr);

    return ((flag & HIME_reply_key_processed) != 0);
}

void hime_im_client_focus_in (HIME_client_handle *handle) {

    if (skip_processing (handle)) {
        return;
    }

    handle->flag |= FLAG_HIME_client_handle_has_focus;

    HIME_req req;
    if (!gen_req (handle, HIME_req_focus_in, &req)) {
        return;
    }

    if (handle_write (handle, &req, sizeof (req)) <= 0) {
        error_proc (handle, "hime_im_client_focus_in error");
    }

    hime_im_client_set_cursor_location (handle, handle->spot_location.x,
                                        handle->spot_location.y);
}

void hime_im_client_focus_out (HIME_client_handle *handle) {

    if (skip_processing (handle)) {
        return;
    }

    handle->flag &= ~FLAG_HIME_client_handle_has_focus;

    HIME_req req;
    if (!gen_req (handle, HIME_req_focus_out, &req)) {
        return;
    }

    if (handle_write (handle, &req, sizeof (req)) <= 0) {
        error_proc (handle, "hime_im_client_focus_out error");
    }
}

// focus out and also read rstr from hime server
void hime_im_client_focus_out2 (HIME_client_handle *handle, char **rstr) {

    if (rstr) {
        *rstr = NULL;
    }

    if (skip_processing (handle)) {
        return;
    }

    handle->flag &= ~FLAG_HIME_client_handle_has_focus;

    HIME_req req;
    if (!gen_req (handle, HIME_req_focus_out2, &req)) {
        return;
    }

    if (handle_write (handle, &req, sizeof (req)) <= 0) {
        error_proc (handle, "hime_im_client_focus_out2 error");
    }

    HIME_reply reply;
    memset (&reply, 0, sizeof (reply));
    if (handle_read (handle, &reply, sizeof (reply)) <= 0) {
        error_proc (handle, "cannot read reply from hime server");
        return;
    }

    to_hime_endian_4 (&reply.datalen);
    to_hime_endian_4 (&reply.flag);

    if (reply.datalen > 0) {
        *rstr = (char *) malloc (reply.datalen);
        if (handle_read (handle, *rstr, reply.datalen) <= 0) {
            free (*rstr);
            *rstr = NULL;
            error_proc (handle, "cannot read reply str from hime server");
            return;
        }
    }
}

void hime_im_client_reset (HIME_client_handle *handle) {

    if (skip_processing (handle)) {
        return;
    }

    HIME_req req;
    if (!gen_req (handle, HIME_req_reset, &req)) {
        return;
    }

    if (handle_write (handle, &req, sizeof (req)) <= 0) {
        error_proc (handle, "hime_im_client_reset error");
    }
}

void hime_im_client_set_cursor_location (HIME_client_handle *handle,
                                         const int x,
                                         const int y) {

    if (skip_processing (handle)) {
        return;
    }

    handle->spot_location.x = x;
    handle->spot_location.y = y;

    if (!BITON (handle->flag, FLAG_HIME_client_handle_has_focus)) {
        return;
    }

    HIME_req req;
    if (!gen_req (handle, HIME_req_set_cursor_location, &req)) {
        return;
    }

    if (handle_write (handle, &req, sizeof (req)) <= 0) {
        error_proc (handle, "hime_im_client_set_cursor_location error");
    }
}

void hime_im_client_set_flags (HIME_client_handle *handle,
                               const int flags,
                               int *ret_flag) {

    if (skip_processing (handle)) {
        return;
    }

    HIME_req req;
    if (!gen_req (handle, HIME_req_set_flags, &req)) {
        return;
    }

    req.flag |= flags;

    flags_backup = req.flag;

    if (handle_write (handle, &req, sizeof (req)) <= 0) {
        error_proc (handle, "hime_im_client_set_flags error");
    }

    if (handle_read (handle, ret_flag, sizeof (int)) <= 0) {
        error_proc (handle, "cannot read ret_flag from hime server");
    }
}

void hime_im_client_clear_flags (HIME_client_handle *handle,
                                 const int flags,
                                 int *ret_flag) {

    if (skip_processing (handle)) {
        return;
    }

    HIME_req req;
    if (!gen_req (handle, HIME_req_set_flags, &req)) {
        return;
    }

    req.flag &= ~flags;

    flags_backup = req.flag;

    if (handle_write (handle, &req, sizeof (req)) <= 0) {
        error_proc (handle, "hime_im_client_clear_flags error");
    }

    if (handle_read (handle, ret_flag, sizeof (int)) <= 0) {
        error_proc (handle, "cannot read ret_flag from hime server");
    }
}

// other APIs

void hime_im_client_send_message (HIME_client_handle *handle,
                                  const char *message) {

    if (!handle || !message) {
        return;
    }

#if DBG
    dbg ("hime_im_client_send_message\n");
#endif
    HIME_req req;
    if (!gen_req (handle, HIME_req_message, &req)) {
        return;
    }

    if (handle_write (handle, &req, sizeof (req)) <= 0) {
        error_proc (handle, "hime_im_client_send_message error w req");
    }

    short len = strlen (message) + 1;
    if (handle_write (handle, &len, sizeof (len)) <= 0) {
        error_proc (handle, "hime_im_client_send_message error w len");
    }

    if (handle_write (handle, message, len) <= 0) {
        error_proc (handle, "hime_im_client_send_message error w message");
    }
}

Window find_hime_window (Display *display) {
    const Atom hime_addr_atom = get_hime_addr_atom (display);
    if (hime_addr_atom == None) {
        return None;
    }
    return XGetSelectionOwner (display, hime_addr_atom);
}
