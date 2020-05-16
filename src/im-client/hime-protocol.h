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

#ifndef HIME_PROTOCOL_H
#define HIME_PROTOCOL_H

#include <X11/Xlib.h>

#include "../hime-endian.h"

#define UNIX_PATH_MAX 108

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
        char dummy[32];  // for future expansion
    };
} HIME_req;

enum {
    HIME_reply_key_processed = 1,
    HIME_reply_key_state_disabled = 2,
};

typedef struct {
    u_int flag;
    u_int datalen;  // '\0' shoule be counted if data is string
} HIME_reply;

#define __HIME_PASSWD_N_ (31)

typedef struct HIME_PASSWD {
    u_int seed;
    u_char passwd[__HIME_PASSWD_N_];
} HIME_PASSWD;

typedef struct {
    u_int ip;
    u_short port;
    HIME_PASSWD passwd;
} Server_IP_port;

typedef struct {
    char sock_path[UNIX_PATH_MAX];
} Server_sock_path;
void __hime_enc_mem (u_char *p, int n, HIME_PASSWD *passwd, u_int *seed);

#endif /* HIME_PROTOCOL_H */
