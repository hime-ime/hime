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
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#ifndef _XSERVER64
#define _XSERVER64
#endif
#else
#include <windows.h>
#include <winsock.h>
#endif
#include "os-dep.h"
#include "hime.h"
#include "hime-protocol.h"
#include "hime-im-client.h"
#define DBG 0
static int flags_backup;

#if UNIX
Atom get_hime_sockpath_atom(Display *dpy);
static void save_old_sigaction_single(int signo, struct sigaction *act)
{
  sigaction(signo, NULL, act);

  if (act->sa_handler != SIG_IGN) {
    signal(signo, SIG_IGN);
  }
}

static void restore_old_sigaction_single(int signo, struct sigaction *act)
{
  if (act->sa_handler != SIG_IGN)
    signal(signo, act->sa_handler);
}
char *get_hime_im_srv_sock_path();
Atom get_hime_addr_atom(Display *dpy);
#endif



#if UNIX
Window find_hime_window(Display *dpy)
{
  Atom hime_addr_atom = get_hime_addr_atom(dpy);
  if (!hime_addr_atom)
    return FALSE;
  return XGetSelectionOwner(dpy, hime_addr_atom);
}
#else
HWND find_hime_window()
{
  return FindWindowA(HIME_WIN_NAME, NULL);
}
#endif


#if WIN32
bool sys_end_session;
HWND serverWnd;


HANDLE open_pipe_client()
{
  int retried=0;
restart:
  serverWnd = find_hime_window();
  if (!serverWnd) {
    if (retried < 10) {
	  if (!retried)
        win32exec("hime.exe");

      Sleep(1000);
	  retried++;
      goto restart;
	} else {
      dbg("exec not found ?\n");
	}

	  MessageBoxA(NULL, "cannot find window", NULL, MB_OK);
	  return NULL;
  }

  dbg("serverwnd %x\n", serverWnd);

  int port = SendMessageA(serverWnd, HIME_PORT_MESSAGE, 0, 0);

  dbg("port %d\n", port);

  char pipe_path[64];
  sprintf(pipe_path, HIME_PIPE_PATH, port);
  dbg("pipe_path %s\n", pipe_path);

  HANDLE hPipe;

  int i;
  for(i=0;i<20;i++)
   {
      hPipe = CreateFileA(
         pipe_path,   // pipe name
         GENERIC_READ |  // read and write access
         GENERIC_WRITE,
         0,              // no sharing
         NULL,           // default security attributes
         OPEN_EXISTING,  // opens existing pipe
         0,              // default attributes
         NULL);          // no template file

   // Break if the pipe handle is valid.

      if (hPipe != INVALID_HANDLE_VALUE) {
		 dbg("connection established %x\n", hPipe);
         return hPipe;
	  }

      // Exit if an error other than ERROR_PIPE_BUSY occurs.

      if (GetLastError() != ERROR_PIPE_BUSY) {
         dbg("Could not open pipe. GLE=%d\n", GetLastError() );
         return NULL;
      }

      // All pipe instances are busy, so wait for 20 seconds.

      if (!WaitNamedPipeA(pipe_path, 2000)) {
         printf("Could not open pipe: 20 second wait timed out.");
         return NULL;
      }
   }

   MessageBoxA(NULL, "cannot connect to hime.exe", NULL, MB_OK);

  return NULL;
}
#endif




static HIME_client_handle *hime_im_client_reopen(HIME_client_handle *hime_ch, Display *dpy)
{
#if WIN32
  char current_exec[80];
  if (GetModuleFileNameA(NULL, current_exec, sizeof(current_exec))) {
	  if (strstr(current_exec, "\\hime.exe"))
		  return NULL;
  }
  int retried;
#endif

//  dbg("hime_im_client_reopen\n");
  int dbg_msg = getenv("HIME_CONNECT_MSG_ON") != NULL;
#if UNIX
  int sockfd=0;
  int servlen;
  char *addr;
  Server_IP_port srv_ip_port;
  u_char *pp;
#else
  HANDLE sockfd;
#endif

  int tcp = FALSE;
  HIME_client_handle *handle;
  int rstatus;

//  dbg("hime_im_client_reopen\n");
#if UNIX
  if (!dpy) {
    dbg("null disp %d\n", hime_ch->fd);
    goto next;
  }

  Atom hime_addr_atom = get_hime_addr_atom(dpy);
  Window hime_win = None;


#define MAX_TRY 3
  int loop;
  for(loop=0; loop < MAX_TRY; loop++) {
    if ((hime_win=find_hime_window(dpy))!=None || getenv("HIME_IM_CLIENT_NO_AUTO_EXEC"))
      break;
    static time_t exec_time;

    if (time(NULL) - exec_time > 1 /* && count < 5 */) {
      time(&exec_time);
      dbg("XGetSelectionOwner: old version of hime or hime is not running ??\n");
      static char execbin[]=HIME_BIN_DIR"/hime";
      dbg("... try to start a new hime server %s\n", execbin);

      int pid;

      if ((pid=fork())==0) {
        putenv("HIME_DAEMON=");
        execl(execbin, "hime", NULL);
      } else {
        int status;
        // hime will daemon()
        waitpid(pid, &status, 0);
      }
    }
  }

  if (loop == MAX_TRY || hime_win == None) {
    goto next;
  }

  Atom actual_type;
  int actual_format;
  u_long nitems,bytes_after;
  char *message_sock = NULL;
  Atom hime_sockpath_atom = get_hime_sockpath_atom(dpy);

//  printf("hime_sockpath_atom %d\n", hime_sockpath_atom);

  if (!hime_sockpath_atom || XGetWindowProperty(dpy, hime_win, hime_sockpath_atom, 0, 64,
     False, AnyPropertyType, &actual_type, &actual_format,
     &nitems,&bytes_after,(u_char **)&message_sock) != Success) {
#if DBG || 1
    dbg("XGetWindowProperty 2: old version of hime or hime is not running ??\n");
#endif
    goto next;
  }

  Server_sock_path srv_sock_path;
  srv_sock_path.sock_path[0] = 0;
  if (message_sock) {
    memcpy(&srv_sock_path, message_sock, sizeof(srv_sock_path));
    XFree(message_sock);
  } else
    goto next;

  struct sockaddr_un serv_addr;
  bzero((char *) &serv_addr,sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  char sock_path[128];

  if (srv_sock_path.sock_path[0]) {
    strcpy(sock_path, srv_sock_path.sock_path);
  }
  else {
    get_hime_im_srv_sock_path(sock_path, sizeof(sock_path));
  }

  addr = sock_path;
  strcpy(serv_addr.sun_path, sock_path);
#ifdef SUN_LEN
  servlen = SUN_LEN(&serv_addr);
#else
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
#endif

  if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("cannot open socket");
    goto tcp;
  }

  if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
    close(sockfd);
    sockfd = 0;
    goto tcp;
  }

  if (dbg_msg)
    dbg("connected to unix socket addr %s\n", sock_path);
  goto next;

  char *message;

tcp:
  message = NULL;

  if (!hime_addr_atom || XGetWindowProperty(dpy, hime_win, hime_addr_atom, 0, 64,
     False, AnyPropertyType, &actual_type, &actual_format,
     &nitems,&bytes_after,(u_char **)&message) != Success) {
#if DBG || 1
    dbg("XGetWindowProperty: old version of hime or hime is not running ??\n");
#endif
    goto next;
  }

  if (message) {
    memcpy(&srv_ip_port, message, sizeof(srv_ip_port));
    XFree(message);
  } else
    goto next;


//  dbg("im server tcp port %d\n", ntohs(srv_ip_port.port));

  struct sockaddr_in in_serv_addr;
  bzero((char *) &in_serv_addr, sizeof(in_serv_addr));

  in_serv_addr.sin_family = AF_INET;
  in_serv_addr.sin_addr.s_addr = srv_ip_port.ip;
  in_serv_addr.sin_port = srv_ip_port.port;
  servlen = sizeof(in_serv_addr);


  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("cannot open socket");
      goto next;
  }

  dbg("sock %d\n", sockfd);

  if (connect(sockfd, (struct sockaddr *)&in_serv_addr, servlen) < 0) {
    dbg("hime_im_client_open cannot open: ") ;
    perror("");
    close(sockfd);
    sockfd = 0;
    goto next;
  }

  pp = (u_char *)&srv_ip_port.ip;
  if (dbg_msg)
    dbg("hime client connected to server %d.%d.%d.%d:%d\n",
        pp[0], pp[1], pp[2], pp[3], ntohs(srv_ip_port.port));
#else
	sockfd = open_pipe_client();
#endif // UNIX

  tcp = TRUE;

next:
  if (!hime_ch)
    handle = tzmalloc(HIME_client_handle, 1);
  else {
    handle = hime_ch;
  }

  if (sockfd < 0)
    sockfd = 0;

  if (sockfd > 0) {
    handle->fd = sockfd;
#if UNIX
    if (tcp) {
      if (!handle->passwd)
        handle->passwd = malloc(sizeof(HIME_PASSWD));
      memcpy(handle->passwd, &srv_ip_port.passwd, sizeof(srv_ip_port.passwd));
    } else {
      if (handle->passwd) {
        free(handle->passwd); handle->passwd = NULL;
      }
    }
#else
	dbg("zzzzz %x\n", sockfd);
	DWORD rn;
	ReadFile(sockfd, &handle->server_idx, sizeof(int), &rn, NULL);
	dbg("hhhhh\n");
	dbg("server_idx %d\n", handle->server_idx);
#endif
  }

  if (handle->fd)  {
    if (BITON(handle->flag, FLAG_HIME_client_handle_has_focus))
      hime_im_client_focus_in(handle);

    hime_im_client_set_flags(handle, flags_backup, &rstatus);
  }

  return handle;
}


static void validate_handle(HIME_client_handle *hime_ch)
{
  if (hime_ch->fd > 0)
    return;

  hime_im_client_reopen(hime_ch, hime_ch->disp);
}


HIME_client_handle *hime_im_client_open(Display *disp)
{
//  dbg("hime_im_client_open\n");
  HIME_client_handle *handle = hime_im_client_reopen(NULL,  disp);
  handle->disp = disp;
  return handle;
}

void hime_im_client_close(HIME_client_handle *handle)
{
  if (!handle)
	  return;
  if (handle->fd > 0)
#if WIN32
    CloseHandle((HANDLE)handle->fd);
#else
    close(handle->fd);
#endif
#if UNIX
  free(handle->passwd);
#endif
  free(handle);
}


static void send_req_msg(HIME_client_handle *handle)
{
#if WIN32
	PostMessage(serverWnd, HIME_CLIENT_MESSAGE_REQ, handle->server_idx, NULL);
#endif
}

static int gen_req(HIME_client_handle *handle, u_int req_no, HIME_req *req)
{
#if WIN32
  if (req_no  & (HIME_req_key_press|HIME_req_key_release|HIME_req_test_key_press|HIME_req_test_key_release)) {
//    dbg("gen_req validate\n");
	validate_handle(handle);
  }
#else
  validate_handle(handle);
#endif

  if (!handle->fd)
    return 0;

  handle->seq++;

  bzero(req, sizeof(HIME_req));

  req->req_no = req_no;
  to_hime_endian_4(&req->req_no);

  req->client_win = handle->client_win;
  to_hime_endian_4(&req->client_win);

  req->input_style = handle->input_style;
  to_hime_endian_4(&req->input_style);

  req->spot_location.x = handle->spot_location.x;
  req->spot_location.y = handle->spot_location.y;
  to_hime_endian_2(&req->spot_location.x);
  to_hime_endian_2(&req->spot_location.y);

  return 1;
}

static void error_proc(HIME_client_handle *handle, char *msg)
{
  if (!handle->fd)
    return;

  perror(msg);
#if WIN32
  CloseHandle(handle->fd);
#else
  close(handle->fd);
#endif
  handle->fd = 0;
#if WIN32
  Sleep(100);
#else
  usleep(100000);
#endif
}


#if WIN32
static int handle_read(HIME_client_handle *handle, void *ptr, int n)
{
  BOOL r;
  HANDLE fd = handle->fd;

  if (!fd)
    return 0;
  DWORD rn;
  r = ReadFile(fd, (char *)ptr, n, &rn, 0);

  if (!r)
	  return -1;

#if (DBG || 0)
  if (r < 0)
    perror("handle_read");
#endif

  return rn;
}
#else
typedef struct {
  struct sigaction apipe;
} SAVE_ACT;
static void save_old_sigaction(SAVE_ACT *save_act)
{
  save_old_sigaction_single(SIGPIPE, &save_act->apipe);
}
static void restore_old_sigaction(SAVE_ACT *save_act)
{
  restore_old_sigaction_single(SIGPIPE, &save_act->apipe);
}
static int handle_read(HIME_client_handle *handle, void *ptr, int n)
{
  int fd = handle->fd;

  if (!fd)
    return 0;

  SAVE_ACT save_act;
  save_old_sigaction(&save_act);
  int r = read(fd, ptr, n);

#if (DBG || 1)
  if (r < 0)
    perror("handle_read");
#endif

  restore_old_sigaction(&save_act);

  if (r<=0)
    return r;
  if (handle->passwd)
    __hime_enc_mem((u_char *)ptr, n, handle->passwd, &handle->passwd->seed);
  return r;
}
#endif


#if WIN32
static int handle_write(HIME_client_handle *handle, void *ptr, int n)
{
  BOOL r;
  char *tmp;
  HANDLE fd = (HANDLE)handle->fd;

  if (!fd)
    return 0;

  tmp = (char *)malloc(n);
  memcpy(tmp, ptr, n);

  DWORD wn;
  r =  WriteFile(fd, tmp, n, &wn, NULL);
  free(tmp);
  if (!r)
	  return -1;
  return wn;
}
#else
static int handle_write(HIME_client_handle *handle, void *ptr, int n)
{
  int fd = handle->fd;

  if (!fd)
    return 0;

  u_char *tmp = malloc(n);
  memcpy(tmp, ptr, n);

  if (handle->passwd)
    __hime_enc_mem(tmp, n, handle->passwd, &handle->passwd->seed);

  SAVE_ACT save_act;
#if 1
  save_old_sigaction(&save_act);
#endif
  int r =  write(fd, tmp, n);
#if 1
  restore_old_sigaction(&save_act);
#endif
  free(tmp);

  return r;
}
#endif


void hime_im_client_focus_in(HIME_client_handle *handle)
{
  if (!handle)
	  return;

  HIME_req req;
//  dbg("hime_im_client_focus_in\n");
  handle->flag |= FLAG_HIME_client_handle_has_focus;

  if (!gen_req(handle, HIME_req_focus_in, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"hime_im_client_focus_in error");
  }
  send_req_msg(handle);

  hime_im_client_set_cursor_location(handle, handle->spot_location.x,
     handle->spot_location.y);
}


void hime_im_client_focus_out(HIME_client_handle *handle)
{
  if (!handle)
    return;

  HIME_req req;
//  dbg("hime_im_client_focus_out\n");
  handle->flag &= ~FLAG_HIME_client_handle_has_focus;

  if (!gen_req(handle, HIME_req_focus_out, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"hime_im_client_focus_out error");
  }

  send_req_msg(handle);
}

#if UNIX
void hime_im_client_focus_out2(HIME_client_handle *handle, char **rstr)
{
  HIME_req req;
  HIME_reply reply;

  if (!handle)
    return;

#if DBG
  dbg("hime_im_client_focus_out2\n");
#endif
  handle->flag &= ~FLAG_HIME_client_handle_has_focus;
  *rstr = NULL;

  if (!gen_req(handle, HIME_req_focus_out2, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"hime_im_client_focus_out error");
  }

  bzero(&reply, sizeof(reply));
  if (handle_read(handle, &reply, sizeof(reply)) <=0) {
    error_proc(handle, "cannot read reply from hime server");
    return;
  }

  to_hime_endian_4(&reply.datalen);
  to_hime_endian_4(&reply.flag);

  if (reply.datalen > 0) {
    *rstr = (char *)malloc(reply.datalen);
    if (handle_read(handle, *rstr, reply.datalen) <= 0) {
      free(*rstr); *rstr = NULL;
      error_proc(handle, "cannot read reply str from hime server");
      return;
    }
  }

//  dbg("hime_im_client_forward_key_event %x\n", reply.flag);

  return;
}
#endif

static int hime_im_client_forward_key_event(HIME_client_handle *handle,
                                          HIME_req_t event_type,
                                          KeySym key, u_int state,
                                          char **rstr)
{
  HIME_reply reply;
  HIME_req req;

  *rstr = NULL;

  if (!gen_req(handle, event_type, &req))
    return 0;

  req.keyeve.key = key;
  to_hime_endian_4(&req.keyeve.key);
  req.keyeve.state = state;
  to_hime_endian_4(&req.keyeve.state);


  if (handle_write(handle, &req, sizeof(req)) <= 0) {
    error_proc(handle, "cannot write to hime server");
    return FALSE;
  }
  send_req_msg(handle);

  bzero(&reply, sizeof(reply));
  if (handle_read(handle, &reply, sizeof(reply)) <=0) {
    error_proc(handle, "cannot read reply from hime server");
    return FALSE;
  }

  to_hime_endian_4(&reply.datalen);
  to_hime_endian_4(&reply.flag);

  if (reply.datalen > 0) {
    *rstr = (char *)malloc(reply.datalen);
    if (handle_read(handle, *rstr, reply.datalen) <= 0) {
      free(*rstr); *rstr = NULL;
      error_proc(handle, "cannot read reply str from hime server");
      return FALSE;
    }
  }

//  dbg("hime_im_client_forward_key_event %x\n", reply.flag);

  return reply.flag;
}


// return TRUE if the key is accepted
int hime_im_client_forward_key_press(HIME_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr)
{
  int flag;
  if (!handle)
    return 0;
  // in case client didn't send focus in event
  if (!BITON(handle->flag, FLAG_HIME_client_handle_has_focus)) {
    hime_im_client_focus_in(handle);
    handle->flag |= FLAG_HIME_client_handle_has_focus;
    hime_im_client_set_cursor_location(handle, handle->spot_location.x,
       handle->spot_location.y);
  }

//  dbg("hime_im_client_forward_key_press\n");
  flag = hime_im_client_forward_key_event(
             handle, HIME_req_key_press, key, state, rstr);

  return ((flag & HIME_reply_key_processed) !=0);
}


// return TRUE if the key is accepted
int hime_im_client_forward_key_release(HIME_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr)
{
  int flag;
  if (!handle)
    return 0;
  handle->flag |= FLAG_HIME_client_handle_has_focus;
//  dbg("hime_im_client_forward_key_release\n");
  flag = hime_im_client_forward_key_event(
             handle, HIME_req_key_release, key, state, rstr);
  return ((flag & HIME_reply_key_processed) !=0);
}


void hime_im_client_set_cursor_location(HIME_client_handle *handle, int x, int y)
{
  if (!handle)
    return;

//  dbg("hime_im_client_set_cursor_location %d   %d,%d\n", handle->flag, x, y);

  HIME_req req;
  handle->spot_location.x = x;
  handle->spot_location.y = y;

  if (!BITON(handle->flag, FLAG_HIME_client_handle_has_focus))
    return;

  if (!gen_req(handle, HIME_req_set_cursor_location, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"hime_im_client_set_cursor_location error");
  }
  send_req_msg(handle);
}

// in win32, if win is NULL, this means hime_im_client_set_cursor_location(x,y) is screen position
void hime_im_client_set_window(HIME_client_handle *handle, Window win)
{
  if (!handle)
	  return;
//  dbg("hime_im_client_set_window %x\n", win);
#if UNIX
  if (!win)
    return;
#endif
  handle->client_win = win;

// For chrome
//  hime_im_client_set_cursor_location(handle, handle->spot_location.x, handle->spot_location.y);
}

void hime_im_client_set_flags(HIME_client_handle *handle, int flags, int *ret_flag)
{
  HIME_req req;

#if DBG
  dbg("hime_im_client_set_flags\n");
#endif

  if (!handle)
    return;

  if (!gen_req(handle, HIME_req_set_flags, &req))
    return;

  req.flag |= flags;

  flags_backup = req.flag;

#if DBG
  dbg("hime_im_client_set_flags b\n");
#endif

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"hime_im_client_set_flags error");
  }
  send_req_msg(handle);

#if DBG
  dbg("hime_im_client_set_flags c\n");
#endif

  if (handle_read(handle, ret_flag, sizeof(int)) <= 0) {
    error_proc(handle, "cannot read reply str from hime server");
  }
}


void hime_im_client_clear_flags(HIME_client_handle *handle, int flags, int *ret_flag)
{
  HIME_req req;

  if (!handle)
    return;

  if (!gen_req(handle, HIME_req_set_flags, &req))
    return;

  req.flag &= ~flags;

  flags_backup = req.flag;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"hime_im_client_set_flags error");
  }
  send_req_msg(handle);

  if (handle_read(handle, ret_flag, sizeof(int)) <= 0) {
    error_proc(handle, "cannot read reply str from hime server");
  }
}


int hime_im_client_get_preedit(HIME_client_handle *handle, char **str, HIME_PREEDIT_ATTR att[], int *cursor
#if WIN32 || 1
    ,int *sub_comp_len
#endif
    )
{
  *str=NULL;
  if (!handle)
    return 0;

  int attN, tcursor, str_len;
#if DBG
  dbg("hime_im_client_get_preedit\n");
#endif
  HIME_req req;
  if (!gen_req(handle, HIME_req_get_preedit, &req)) {
err_ret:
#if DBG
    dbg("aaaaaaaaaaaaa %x\n", str);
#endif
    if (cursor)
      *cursor=0;
    *str=strdup("");
    return 0;
  }

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"hime_im_client_get_preedit error");
    goto err_ret;
  }
  send_req_msg(handle);

  str_len=-1; // str_len includes \0
  if (handle_read(handle, &str_len, sizeof(str_len))<=0)
    goto err_ret; // including \0

  *str = (char *)malloc(str_len);

  if (handle_read(handle, *str, str_len)<=0)
    goto err_ret;
#if DBG
  dbg("hime_im_client_get_preedit len:%d '%s' \n", str_len, *str);
#endif
  attN = -1;
  if (handle_read(handle, &attN, sizeof(attN))<=0) {
    goto err_ret;
  }

//  dbg("attrN:%d\n", attN);

  if (attN>0 && handle_read(handle, att, sizeof(HIME_PREEDIT_ATTR)*attN)<=0) {
    goto err_ret;
  }


  tcursor=0;
  if (handle_read(handle, &tcursor, sizeof(tcursor))<=0) {
    goto err_ret;
  }

  if (cursor)
    *cursor = tcursor;


#if WIN32 || 1
  int tsub_comp_len;
  tsub_comp_len=0;
  if (handle_read(handle, &tsub_comp_len, sizeof(tsub_comp_len))<=0) {
    goto err_ret;
  }
  if (sub_comp_len)
	*sub_comp_len = tsub_comp_len;
#endif

#if DBG
  dbg("jjjjjjjjj %d tcursor:%d\n", attN, tcursor);
#endif
  return attN;
}



void hime_im_client_reset(HIME_client_handle *handle)
{
  if (!handle)
	  return;

  HIME_req req;
#if DBG
  dbg("hime_im_client_reset\n");
#endif
  if (!gen_req(handle, HIME_req_reset, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"hime_im_client_reset error");
  }
  send_req_msg(handle);
}


void hime_im_client_message(HIME_client_handle *handle, char *message)
{
  HIME_req req;
  short len;
#if DBG
  dbg("hime_im_client_message\n");
#endif
  if (!gen_req(handle, HIME_req_message, &req))
    return;

  if (handle_write(handle, &req, sizeof(req)) <=0) {
    error_proc(handle,"hime_im_client_message error 1");
  }
  send_req_msg(handle);

  len = strlen(message)+1;
  if (handle_write(handle, &len, sizeof(len)) <=0) {
    error_proc(handle,"hime_im_client_message error 2");
  }

  if (handle_write(handle, message, len) <=0) {
    error_proc(handle,"hime_im_client_message error 2");
  }
}


#if TSF
bool hime_im_client_key_eaten(HIME_client_handle *handle, int press_release,
                                          KeySym key, u_int state)
{
  HIME_reply reply;
  HIME_req req;

  if (!gen_req(handle, press_release?HIME_req_test_key_release:HIME_req_test_key_press, &req))
    return 0;

  req.keyeve.key = key;
  to_hime_endian_4(&req.keyeve.key);
  req.keyeve.state = state;
  to_hime_endian_4(&req.keyeve.state);


  if (handle_write(handle, &req, sizeof(req)) <= 0) {
    error_proc(handle, "cannot write to hime server");
    return FALSE;
  }
  send_req_msg(handle);

  bzero(&reply, sizeof(reply));
  if (handle_read(handle, &reply, sizeof(reply)) <=0) {
    error_proc(handle, "cannot read reply from hime server");
    return FALSE;
  }

  to_hime_endian_4(&reply.datalen);
  to_hime_endian_4(&reply.flag);

  return (reply.flag & HIME_reply_key_processed) > 0;
}
#endif
