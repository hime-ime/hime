#if UNIX
#include <X11/Xlib.h>
#endif
#include "../hime-endian.h"

typedef enum {
  HIME_req_key_press = 1,
  HIME_req_key_release = 2,
  HIME_req_focus_in = 4,
  HIME_req_focus_out = 8,
  HIME_req_set_cursor_location = 0x10,
  HIME_req_set_flags = 0x20,
  HIME_req_get_preedit = 0x40,
  HIME_req_reset = 0x80,
  HIME_req_focus_out2 = 0x100,
  HIME_req_message = 0x200,
  HIME_req_test_key_press = 0x400,
  HIME_req_test_key_release = 0x800,
} HIME_req_t;


typedef struct {
#if 0
    KeySym key;
#else
    u_int key;
#endif
    u_int state;
} KeyEvent;

typedef struct {
    short x, y;
} HIMEpoint;


typedef struct {
  u_int req_no;  // to make the im server stateless, more is better
  u_int client_win;
  u_int flag;
  u_int input_style;
  HIMEpoint spot_location;

  union {
    KeyEvent keyeve;
    char dummy[32];   // for future expansion
  };
} HIME_req;


enum {
  HIME_reply_key_processed = 1,
  HIME_reply_key_state_disabled = 2,
};


typedef struct {
  u_int flag;
  u_int datalen;    // '\0' shoule be counted if data is string
} HIME_reply;


#define __HIME_PASSWD_N_ (31)

#if !WIN32
typedef struct HIME_PASSWD {
  u_int seed;
  u_char passwd[__HIME_PASSWD_N_];
} HIME_PASSWD;
#endif

typedef struct {
  u_int ip;
  u_short port;
#if !WIN32
  HIME_PASSWD passwd;
#endif
} Server_IP_port;

typedef struct {
  char sock_path[80];
} Server_sock_path;
#if UNIX
void __hime_enc_mem(u_char *p, int n, HIME_PASSWD *passwd, u_int *seed);
#endif

#if WIN32
#define HIME_WIN_NAME "hime0"
#define HIME_PORT_MESSAGE WM_USER+10
#define HIME_CLIENT_MESSAGE_REQ WM_USER+11
#define HIME_PIPE_PATH "\\\\.\\pipe\\hime-svr%d"
#endif
