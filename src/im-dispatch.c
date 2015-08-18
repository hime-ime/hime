/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include "hime.h"
#include "hime-protocol.h"
#include "hime-im-client.h"
#include "im-srv.h"
#include <gtk/gtk.h>

#define DBG 0

static int myread(int fd, void *buf, int bufN)
{
  return read(fd, buf, bufN);
}


HIME_ENT *hime_clients;
int hime_clientsN;

extern HIME_PASSWD my_passwd;

gboolean ProcessKeyPress(KeySym keysym, u_int kev_state);
gboolean ProcessTestKeyPress(KeySym keysym, u_int kev_state);
gboolean ProcessKeyRelease(KeySym keysym, u_int kev_state);
gboolean ProcessTestKeyRelease(KeySym keysym, u_int kev_state);
int hime_FocusIn(ClientState *cs);
int hime_FocusOut(ClientState *cs);
void update_in_win_pos();
void hide_in_win(ClientState *cs);
void init_state_chinese(ClientState *cs);
void clear_output_buffer();
void flush_edit_buffer();
int hime_get_preedit(ClientState *cs, char *str, HIME_PREEDIT_ATTR attr[], int *cursor, int *sub_comp_len);
void hime_reset();
void dbg_time(char *fmt,...);

extern char *output_buffer;
extern int output_bufferN;

int write_enc(int fd, void *p, int n)
{
  if (!fd)
    return 0;

  unsigned char *tmp = (unsigned char *)malloc(n);
  memcpy(tmp, p, n);
  if (hime_clients[fd].type == Connection_type_tcp) {
    __hime_enc_mem(tmp, n, &srv_ip_port.passwd, &hime_clients[fd].seed);
  }
  int r =  write(fd, tmp, n);

#if DBG
  if (r < 0)
    perror("write_enc");
#endif

  free(tmp);

  return r;
}

#include <pwd.h>

static void shutdown_client(int fd)
{
//  dbg("client shutdown rn %d\n", rn);
  g_source_remove(hime_clients[fd].tag);
  int idx = fd;

  if (hime_clients[idx].cs == current_CS) {
    hide_in_win(current_CS);
    current_CS = NULL;
  }

  free(hime_clients[idx].cs);
  hime_clients[idx].cs = NULL;
  hime_clients[idx].fd = 0;

/* Now we use "is_special_user" */
#if 0
  int uid = getuid();
  struct passwd *pwd;
  if ((pwd=getpwuid(uid)) &&
      (!strcmp(pwd->pw_name, "gdm") || !strcmp(pwd->pw_name, "kdm"))) {
    exit(0);
  }
#endif
  close(fd);
}

void message_cb(char *message);
void save_CS_temp_to_current();

static gboolean is_init_im_enabled = FALSE;


inline void parse_client_req(HIME_req* req)
{
  to_hime_endian_4(&req.req_no);
  to_hime_endian_4(&req.client_win);
  to_hime_endian_4(&req.flag);
  to_hime_endian_2(&req.spot_location.x);
  to_hime_endian_2(&req.spot_location.y);
}

void process_client_req(int fd)
{
  HIME_req req;
#if DBG
  dbg("svr--> process_client_req %d\n", fd);
#endif
  int rn = myread(fd, &req, sizeof(req));

  if (rn <= 0) {
    shutdown_client(fd);
    return;
  }
  if (hime_clients[fd].type == Connection_type_tcp) {
    __hime_enc_mem((u_char *)&req, sizeof(req), &srv_ip_port.passwd, &hime_clients[fd].seed);
  }

  parse_client_req(&req);
//  dbg("spot %d %d\n", req.spot_location.x, req.spot_location.y);

  ClientState *cs = NULL;

  if (current_CS && req.client_win == current_CS->client_win) {
    cs = current_CS;
  } else {
    int idx = fd;
    cs = hime_clients[fd].cs;

    int new_cli = 0;
    if (!cs) {
      cs = hime_clients[idx].cs = tzmalloc(ClientState, 1);
      new_cli = 1;
    }

    cs->client_win = req.client_win;
    cs->b_hime_protocol = TRUE;
    cs->input_style = InputStyleOverSpot;

    if (hime_init_im_enabled && ((hime_single_state && !is_init_im_enabled) || (!hime_single_state && new_cli))) {
      dbg("new_cli default_input_method:%d\n", default_input_method);
      is_init_im_enabled = TRUE;
      current_CS = cs;
      save_CS_temp_to_current();
      init_state_chinese(cs);
    }
  }

  if (!cs)
    p_err("bad cs\n");

  if (req.req_no != HIME_req_message) {
    cs->spot_location.x = req.spot_location.x;
    cs->spot_location.y = req.spot_location.y;
  }

  gboolean status;
  HIME_reply reply;
  bzero(&reply, sizeof(reply));

  switch (req.req_no) {
    case HIME_req_key_press:
    case HIME_req_key_release:
      current_CS = cs;
      save_CS_temp_to_current();

#if DBG && 0
      {
        char tt[128];

        if (req.keyeve.key < 127) {
          sprintf(tt,"'%c'", req.keyeve.key);
        } else {
          strcpy(tt, XKeysymToString(req.keyeve.key));
        }

        dbg_time("HIME_key_press  %x %s\n", cs, tt);
      }
#endif
      to_hime_endian_4(&req.keyeve.key);
      to_hime_endian_4(&req.keyeve.state);

//	  dbg("serv key eve %x %x predit:%d\n",req.keyeve.key, req.keyeve.state, cs->use_preedit);

#if DBG
	  char *typ;
      typ="press";
#endif

      if (req.req_no==HIME_req_key_press)
        status = ProcessKeyPress(req.keyeve.key, req.keyeve.state);
      else {
        status = ProcessKeyRelease(req.keyeve.key, req.keyeve.state);


#if DBG
        typ="rele";
#endif
      }

      if (status)
        reply.flag |= HIME_reply_key_processed;
#if DBG
      dbg("%s srv flag:%x status:%d len:%d %x %c\n",typ, reply.flag, status, output_bufferN, req.keyeve.key,req.keyeve.key & 0x7f);
#endif
      int datalen;
      datalen = reply.datalen =
        output_bufferN ? output_bufferN + 1 : 0; // include '\0'
      to_hime_endian_4(&reply.flag);
      to_hime_endian_4(&reply.datalen);
      write_enc(fd, &reply, sizeof(reply));

//      dbg("server reply.flag %x\n", reply.flag);

      if (output_bufferN) {
        write_enc(fd, output_buffer, datalen);
        clear_output_buffer();
      }

      break;
    case HIME_req_focus_in:
#if DBG
      dbg_time("HIME_req_focus_in  %x %d %d\n",cs, cs->spot_location.x, cs->spot_location.y);
#endif
//      current_CS = cs;
      hime_FocusIn(cs);
      break;
    case HIME_req_focus_out:
#if DBG
      dbg_time("HIME_req_focus_out  %x\n", cs);
#endif
      hime_FocusOut(cs);
      break;
    case HIME_req_focus_out2:
      {
#if DBG
      dbg_time("HIME_req_focus_out2  %x\n", cs);
#endif
      if (hime_FocusOut(cs))
        flush_edit_buffer();

      HIME_reply reply;
      bzero(&reply, sizeof(reply));

      int datalen = reply.datalen =
        output_bufferN ? output_bufferN + 1 : 0; // include '\0'
      to_hime_endian_4(&reply.flag);
      to_hime_endian_4(&reply.datalen);
      write_enc(fd, &reply, sizeof(reply));

//      dbg("server reply.flag %x\n", reply.flag);

      if (output_bufferN) {
        write_enc(fd, output_buffer, datalen);
        clear_output_buffer();
      }
      }
      break;
    case HIME_req_set_cursor_location:
#if DBG
      dbg_time("set_cursor_location %x %d %d\n", cs,
         cs->spot_location.x, cs->spot_location.y);
#endif
      update_in_win_pos();
      break;
    case HIME_req_set_flags:
//      dbg("HIME_req_set_flags\n");
      if (BITON(req.flag, FLAG_HIME_client_handle_raise_window)) {
#if DBG
        dbg("********* raise * window\n");
#endif
        if (!hime_pop_up_win)
          cs->b_raise_window = TRUE;
      }

	  if (req.flag & FLAG_HIME_client_handle_use_preedit)
        cs->use_preedit = TRUE;

      int rflags;
      rflags = 0;
      if (hime_pop_up_win)
        rflags = FLAG_HIME_srv_ret_status_use_pop_up;

      write_enc(fd, &rflags, sizeof(rflags));
      break;
    case HIME_req_get_preedit:
      {
#if DBG
      dbg("svr HIME_req_get_preedit %x\n", cs);
#endif
      char str[HIME_PREEDIT_MAX_STR];
      HIME_PREEDIT_ATTR attr[HIME_PREEDIT_ATTR_MAX_N];
      int cursor, sub_comp_len;
      int attrN = hime_get_preedit(cs, str, attr, &cursor, &sub_comp_len);
      if (hime_edit_display&(HIME_EDIT_DISPLAY_BOTH|HIME_EDIT_DISPLAY_OVER_THE_SPOT))
        cursor=0;
      if (hime_edit_display&HIME_EDIT_DISPLAY_OVER_THE_SPOT) {
        attrN=0;
        str[0]=0;
      }
      int len = strlen(str)+1; // including \0
      write_enc(fd, &len, sizeof(len));
      write_enc(fd, str, len);
//      dbg("attrN:%d\n", attrN);
      write_enc(fd, &attrN, sizeof(attrN));
      if (attrN > 0)
        write_enc(fd, attr, sizeof(HIME_PREEDIT_ATTR)*attrN);
        write_enc(fd, &cursor, sizeof(cursor));
        write_enc(fd, &sub_comp_len, sizeof(sub_comp_len));
//      dbg("uuuuuuuuuuuuuuuuu len:%d %d cursor:%d\n", len, attrN, cursor);
      }
      break;
    case HIME_req_reset:
      hime_reset();
      break;
    case HIME_req_message:
      {
//        dbg("HIME_req_message\n");
        short len=0;
        int rn = myread(fd, &len, sizeof(len));
        if (rn <= 0) {
cli_down:
          shutdown_client(fd);
          return;
        }

        // only unix socket, no decrypt
        char buf[512];
        // message should include '\0'
        if (len > 0 && len < sizeof(buf)) {
          if (myread(fd, buf, len)<=0)
            goto cli_down;
          message_cb(buf);
        }
      }
      break;
    default:
      dbg_time("Invalid request %x from:", req.req_no);
      struct sockaddr_in addr;
      socklen_t len=sizeof(addr);
      bzero(&addr, sizeof(addr));

      if (!getpeername(fd, (struct sockaddr *)&addr, &len)) {
        dbg("%s\n", inet_ntoa(addr.sin_addr));
      } else {
        perror("getpeername\n");
      }
      shutdown_client(fd);
      break;
  }
}

