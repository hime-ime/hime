/* Copyright (C) 2011-2012 cwlin <https://github.com/cwlin>
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

#ifndef CHEWING_H
#define CHEWING_H

#include <chewing.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#include "hime.h"
#include "pho.h"
#include "gst.h"
#include "im-client/hime-im-client-attr.h"
#include "hime-module.h"
#include "hime-module-cb.h"
#include "hime-conf.h"

#define MAX_SEG_NUM  128
#define HIME_CHEWING_CONFIG "/.config/hime/config/chewing_conf.dat"
#define HIME_KB_CONFIG "/.config/hime/config/phonetic-keyboard2"

#define HIME_CHEWING_DEFAULT_SELECT_KEYS { '1', '2', '3', '4', \
                                           '5', '6', '7', '8', \
                                           '9', '0' }
#define HIME_CHEWING_DEFAULT_NUMBER_OF_SELECT_KEYS 10

#define HIME_CHEWING_DEFAULT_KEY_MIN (XK_space)
#define HIME_CHEWING_DEFAULT_KEY_MAX (XK_asciitilde + 1)
#define HIME_CHEWING_KEY_MIN         (0x00)
#define HIME_CHEWING_KEY_MAX         (XK_Delete + 1)

#define HIME_CHEWING_WRAPPER_FUNC(FUNC_NAME) return (chewing_buffer_Len (g_pChewingCtx) \
                                             == 0 ? (-1) : FUNC_NAME(g_pChewingCtx));

typedef struct _SEGMENT 
{
    GtkWidget *label;
    unsigned char selidx, selN;
} SEG;

typedef struct KB_MAPPING
{
    char *pszHimeKbName;
    char *pszChewingKbName;
} kbmapping_t;

// hime module callbacks
int module_init_win (HIME_module_main_functions *pFuncs);
void module_get_win_geom (void);
int module_reset (void);
int module_get_preedit (char *pszStr, HIME_PREEDIT_ATTR attr[],
                        int *pnCursor, int *pCompFlag);
gboolean module_feedkey (int nKeyVal, int nKeyState);
int module_feedkey_release (KeySym xkey, int nKbState);
void module_move_win(int x, int y);
void module_change_font_size (void);
void module_show_win (void);
void module_hide_win (void);
int module_win_visible (void);
int module_flush_input (void);

// config funcs
void chewing_config_open (gboolean bWrite);
void chewing_config_load (ChewingConfigData *pChewingConfig);
void chewing_config_set (ChewingContext *pChewingContext);
void chewing_config_dump (void);
void chewing_config_close (void);
gboolean chewing_config_save (int nVal[]);

#endif
