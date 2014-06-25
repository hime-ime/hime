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

#ifndef HIME_IM_CLIENT_H
#define HIME_IM_CLIENT_H

#ifndef _XSERVER64
#define _XSERVER64
#endif

struct HIME_PASSWD;

typedef struct HIME_client_handle_S {
  int fd;
  Window client_win;	/* client window */
  u_int	input_style;	/* input style */
  XPoint spot_location; /* spot location */
// below is private data, don't modify them.
  u_int flag;
  Display *disp;
  struct HIME_PASSWD *passwd;
  u_int seq;
} HIME_client_handle;

enum {
  FLAG_HIME_client_handle_has_focus = 1,
  FLAG_HIME_client_handle_use_preedit = 2,
  FLAG_HIME_client_handle_raise_window = 0x1000  // for mozilla, dirty fix
};

enum {
  FLAG_HIME_srv_ret_status_use_pop_up = 1    // If this is used, we don't need the dirty fix
};


#ifdef __cplusplus
extern "C" {
#endif

HIME_client_handle *hime_im_client_open(Display *disp);
void hime_im_client_close(HIME_client_handle *handle);
void hime_im_client_focus_in(HIME_client_handle *handle);
void hime_im_client_focus_out(HIME_client_handle *handle);
void hime_im_client_focus_out2(HIME_client_handle *handle, char **rstr);
void hime_im_client_set_window(HIME_client_handle *handle, Window win);
void hime_im_client_set_cursor_location(HIME_client_handle *handle,
                                        int x, int y);
/*  rstr returns UTF-8 encoded string, you should use 'free()' to free the
    memory.

    return boolean:
      FALSE : the key is rejected, should use client's own result(ASCII key).
      TRUE : the key is accepted, translated result is in rstr.
 */
int hime_im_client_forward_key_press(HIME_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr);
// return some state bits instead of TRUE/FALSE
int hime_im_client_forward_key_press2(HIME_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr);
int hime_im_client_forward_key_release(HIME_client_handle *handle,
                                          KeySym key, u_int state,
                                          char **rstr);

void hime_im_client_set_flags(HIME_client_handle *handle, int flags, int *ret_flags);
void hime_im_client_clear_flags(HIME_client_handle *handle, int flags, int *ret_flags);

void hime_im_client_reset(HIME_client_handle *handle);
void hime_im_client_message(HIME_client_handle *handle, char *message);

#include "hime-im-client-attr.h"
int hime_im_client_get_preedit(HIME_client_handle *handle, char **str, HIME_PREEDIT_ATTR att[], int *cursor, int *sub_comp_len);

Window find_hime_window(Display *dpy);

#ifdef __cplusplus
}
#endif


#endif
