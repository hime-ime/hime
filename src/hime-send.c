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

#include "hime.h"
#include "hime-im-client.h"

#if UNIX
void send_hime_message(Display *dpy, char *s)
#else
void send_hime_message(char *s)
#endif
{
#if UNIX
  HIME_client_handle *handle = hime_im_client_open(dpy);
#else
  HIME_client_handle *handle = hime_im_client_open(NULL);
#endif
  hime_im_client_message(handle, s);

  hime_im_client_close(handle);
}
