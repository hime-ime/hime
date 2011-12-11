/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#if UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <string.h>
#include "hime.h"
#include "hime-protocol.h"
#include "hime-im-client.h"
#include "im-srv.h"
#include <gtk/gtk.h>

#define DBG 0

#if UNIX
static int myread(int fd, void *buf, int bufN)
#else
static int myread(HANDLE fd, void *buf, int bufN)
#endif
{
#if UNIX
  return read(fd, buf, bufN);
#else
  int ofs=0, toN = bufN;
  while (toN) {
	DWORD rn;
    BOOL r = ReadFile(fd, ((char *)buf) + ofs, toN, &rn, 0);
    if (!r)
      return -1;
    ofs+=rn;
	toN-=rn;
  };
  return bufN;
#endif
}


HIME_ENT *hime_clients;
int hime_clientsN;

#if WIN32
// need to use Enter/Leave CriticalSection in the future
int find_im_client(HANDLE hand)
{
	int i;
	for(i=0;i<hime_clientsN;i++)
		if (hime_clients[i].fd == hand)
			break;
	if (i==hime_clientsN)
		return -1;
	return i;
}

int add_im_client(HANDLE hand)
{
	int i = find_im_client(0);
	if (i<0) {
		hime_clients=trealloc(hime_clients, HIME_ENT, hime_clientsN);
		i=hime_clientsN++;
	}

	ZeroMemory(&hime_clients[i], sizeof(HIME_ENT));
	hime_clients[i].fd = hand;
	return i;
}

#endif

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

#if UNIX
int write_enc(int fd, void *p, int n)
#else
int write_enc(HANDLE fd, void *p, int n)
#endif
{
#if WIN32
  DWORD wn;
  BOOL r = WriteFile(fd, (char *)p, n, &wn, 0);
  if (!r) {
    perror("write_enc");
	return -1;
  }
  return wn;
#else
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
#endif
}

#ifdef __cplusplus
extern "C" void gdk_input_remove	  (gint		     tag);
#endif

#if WIN32
typedef int socklen_t;
#else
#include <pwd.h>
#endif

#if UNIX
static void shutdown_client(int fd)
#else
static void shutdown_client(HANDLE fd)
#endif
{
//  dbg("client shutdown rn %d\n", rn);
#if UNIX
  g_source_remove(hime_clients[fd].tag);
  int idx = fd;
#else
  int idx = find_im_client(fd);
#endif

  if (hime_clients[idx].cs == current_CS) {
    hide_in_win(current_CS);
    current_CS = NULL;
  }

  free(hime_clients[idx].cs);
  hime_clients[idx].cs = NULL;
#if UNIX
  hime_clients[idx].fd = 0;
#else
  hime_clients[idx].fd = NULL;
#endif

#if UNIX
  int uid = getuid();
  struct passwd *pwd;
  if ((pwd=getpwuid(uid)) &&
      (!strcmp(pwd->pw_name, "gdm") || !strcmp(pwd->pw_name, "kdm"))) {
    exit(0);
  }
#endif
#if UNIX
  close(fd);
#else
  CloseHandle(fd);
//  CloseHandle(handle);
#endif
}

void message_cb(char *message);
void save_CS_temp_to_current();


#if UNIX
void process_client_req(int fd)
#else
void process_client_req(HANDLE fd)
#endif
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
#if UNIX
  if (hime_clients[fd].type == Connection_type_tcp) {
    __hime_enc_mem((u_char *)&req, sizeof(req), &srv_ip_port.passwd, &hime_clients[fd].seed);
  }
#endif
  to_hime_endian_4(&req.req_no);
  to_hime_endian_4(&req.client_win);
  to_hime_endian_4(&req.flag);
  to_hime_endian_2(&req.spot_location.x);
  to_hime_endian_2(&req.spot_location.y);

//  dbg("spot %d %d\n", req.spot_location.x, req.spot_location.y);

  ClientState *cs = NULL;

  if (current_CS && req.client_win == current_CS->client_win) {
    cs = current_CS;
  } else {
#if UNIX
    int idx = fd;
    cs = hime_clients[fd].cs;
#else
    int idx = find_im_client(fd);
    cs = hime_clients[idx].cs;
#endif

    int new_cli = 0;
    if (!cs) {
      cs = hime_clients[idx].cs = tzmalloc(ClientState, 1);
      new_cli = 1;
    }

    cs->client_win = req.client_win;
    cs->b_hime_protocol = TRUE;
    cs->input_style = InputStyleOverSpot;


#if WIN32
    cs->use_preedit = TRUE;
#endif

#if UNIX
    if (hime_init_im_enabled && new_cli)
#else
    if (new_cli)
#endif
    {
      current_CS = cs;
      dbg("new_cli default_input_method:%d\n", default_input_method);
      save_CS_temp_to_current();
      init_state_chinese(cs);
      cs->in_method = default_input_method;
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
#if 0
      if (req.req_no==HIME_req_key_press)
        status = Process2KeyPress(req.keyeve.key, req.keyeve.state);
      else {
        status = Process2KeyRelease(req.keyeve.key, req.keyeve.state);
#else
      if (req.req_no==HIME_req_key_press)
        status = ProcessKeyPress(req.keyeve.key, req.keyeve.state);
      else {
        status = ProcessKeyRelease(req.keyeve.key, req.keyeve.state);
#endif

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
#if WIN32
    case HIME_req_test_key_press:
    case HIME_req_test_key_release:
      current_CS = cs;
      save_CS_temp_to_current();
      to_hime_endian_4(&req.keyeve.key);
      to_hime_endian_4(&req.keyeve.state);

//	  dbg("serv key eve %x %x predit:%d\n",req.keyeve.key, req.keyeve.state, cs->use_preedit);

      if (req.req_no==HIME_req_test_key_press)
        status = ProcessTestKeyPress(req.keyeve.key, req.keyeve.state);
      else
        status = ProcessTestKeyRelease(req.keyeve.key, req.keyeve.state);

      if (status)
        reply.flag |= HIME_reply_key_processed;

      reply.datalen = 0;
      to_hime_endian_4(&reply.flag);
      to_hime_endian_4(&reply.datalen);
      write_enc(fd, &reply, sizeof(reply));
      break;
#endif
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
#if UNIX
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
#endif
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
#if WIN32 || 1
      write_enc(fd, &sub_comp_len, sizeof(sub_comp_len));
#endif
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
#if UNIX
      struct sockaddr_in addr;
      socklen_t len=sizeof(addr);
      bzero(&addr, sizeof(addr));

      if (!getpeername(fd, (struct sockaddr *)&addr, &len)) {
        dbg("%s\n", inet_ntoa(addr.sin_addr));
      } else {
        perror("getpeername\n");
      }
#endif
      shutdown_client(fd);
      break;
  }
}

