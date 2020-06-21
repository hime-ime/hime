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

#ifndef HIME_IM_SRV_H
#define HIME_IM_SRV_H

#include <pwd.h>

#include <sys/stat.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "hime-protocol.h"

typedef enum {
    Connection_type_unix = 1,
    Connection_type_tcp = 2
} Connection_type;

typedef struct {
    ClientState *cs;
    int tag;
    uint32_t seed;
    Connection_type type;
    int fd;
} HIME_ENT;

// im-addr.c
Atom get_hime_addr_atom (Display *display);
Atom get_hime_sockpath_atom (Display *display);
void get_hime_im_srv_sock_path (char *outstr, const int outstrN);

// im-dispatch.c
extern HIME_ENT *hime_clients;
extern int hime_clientsN;
void process_client_req (const int fd);

// im-srv.c
extern Server_IP_port srv_ip_port;
void init_hime_im_serv (const Window window);

#endif /* HIME_IM_SRV_H */
