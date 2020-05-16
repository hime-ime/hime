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

/**
 @file hime-module-cb.c
 @brief Handle module callback.

 Find hime modules.
*/

#include <dlfcn.h>

#include "hime.h"

#include "hime-module-cb.h"

#define SETUP_CB(fn)                                                    \
    do {                                                                \
        *(void **) (&st.fn) = dlsym (handle, #fn);                      \
        if (st.fn == NULL) {                                            \
            dbg ("[W] %s doesn't provide callback: %s\n", sofile, #fn); \
        }                                                               \
    } while (0)

HIME_module_callback_functions *init_HIME_module_callback_functions (char *sofile) {
    void *handle;
    char *error;
    char so_absolute_path[512];
    char *module_path = getenv ("HIME_MODULE_DIR");
    if (module_path)
        g_snprintf (so_absolute_path, sizeof (so_absolute_path), "%s/%s", module_path, sofile);
    else
        g_snprintf (so_absolute_path, sizeof (so_absolute_path), "%s", sofile);

    if (!(handle = dlopen (so_absolute_path, RTLD_LAZY))) {
        if ((error = dlerror ()) != NULL) {
            fprintf (stderr, "%s\n", error);
        }
        dbg ("dlopen %s failed\n", sofile);
        return NULL;
    }

    HIME_module_callback_functions st;
    *(void **) (&st.module_init_win) = dlsym (handle, "module_init_win");
    if (!st.module_init_win)
        p_err ("module_init_win() not found in %s", sofile);

    SETUP_CB (module_get_win_geom);
    SETUP_CB (module_reset);
    SETUP_CB (module_get_preedit);
    SETUP_CB (module_feedkey);
    SETUP_CB (module_feedkey_release);
    SETUP_CB (module_move_win);
    SETUP_CB (module_change_font_size);
    SETUP_CB (module_show_win);
    SETUP_CB (module_hide_win);
    SETUP_CB (module_win_visible);
    SETUP_CB (module_flush_input);
    SETUP_CB (module_setup_window_create);

    return tmemdup (&st, HIME_module_callback_functions, 1);
}
