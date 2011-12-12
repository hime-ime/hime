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
#include "gtab.h"
#include "pho.h"
#include "tsin.h"
#include "gst.h"
#include "im-client/hime-im-client-attr.h"
#include "win1.h"
#include "hime-module.h"
#include "hime-module-cb.h"

#if UNIX
#include <dlfcn.h>
#endif

HIME_module_callback_functions *init_HIME_module_callback_functions(char *sofile)
{
#if UNIX
  void *handle;
  char *error;

  if (!(handle = dlopen(sofile, RTLD_LAZY))) {
    if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
    }
    dbg("dlopen %s failed\n", sofile);
    return NULL;
  }
#else
  HMODULE handle = LoadLibraryA(sofile);
  if (!handle)
    return NULL;
#define dlsym GetProcAddress
#endif

  HIME_module_callback_functions st;
  *(void **) (&st.module_init_win) = dlsym(handle, "module_init_win");
  if (!st.module_init_win)
    p_err("module_init_win() not found in %s", sofile);

  *(void **) (&st.module_get_win_geom) = dlsym(handle, "module_get_win_geom");
  *(void **) (&st.module_reset) = dlsym(handle, "module_reset");
  *(void **) (&st.module_get_preedit) = dlsym(handle, "module_get_preedit");
  *(void **) (&st.module_feedkey) = dlsym(handle, "module_feedkey");
  *(void **) (&st.module_feedkey_release) = dlsym(handle, "module_feedkey_release");
  *(void **) (&st.module_move_win) = dlsym(handle, "module_move_win");
  *(void **) (&st.module_change_font_size) = dlsym(handle, "module_change_font_size");
  *(void **) (&st.module_show_win) = dlsym(handle, "module_show_win");
  *(void **) (&st.module_hide_win) = dlsym(handle, "module_hide_win");
  *(void **) (&st.module_win_visible) = dlsym(handle, "module_win_visible");
  *(void **) (&st.module_flush_input) = dlsym(handle, "module_flush_input");
  *(void **) (&st.module_setup_window_create) = dlsym(handle, "module_setup_window_create");

  return tmemdup(&st, HIME_module_callback_functions, 1);
}
