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

typedef enum {
  Connection_type_unix = 1,
  Connection_type_tcp = 2
} Connection_type;

typedef struct {
  ClientState *cs;
  int tag;
  u_int seed;
  Connection_type type;
#if	UNIX
  int fd;
#else
  HANDLE fd;
#endif
} HIME_ENT;

extern HIME_ENT *hime_clients;
extern int hime_clientsN;
extern Server_IP_port srv_ip_port;
