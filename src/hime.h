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

#ifndef HIME_H
#define HIME_H

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#if !GTK_CHECK_VERSION(3, 0, 0)
#include <gdk/gdkkeysyms.h>
#endif

#include <X11/Xlib.h>

/* To get GETTEXT_PACKAGE */
#include "config.h"

#if HIME_I18N_MESSAGE
#include <libintl.h>
#define _(STRING) gettext (STRING)
#else
#define _(STRING) (STRING)
#endif
#define N_(STRING) (STRING)

#include "IMdkit.h"
#include "Xi18n.h"
#include "os-dep.h"

typedef enum {
    HIME_STATE_DISABLED = 0,
    HIME_STATE_ENG_FULL = 1,
    HIME_STATE_CHINESE = 2
} HIME_STATE_E;

/* change 3 to 4 if you want to use 4-byte UTF-8 characters, but you must
   regenerate *.gtab tsin
*/
#define CH_SZ (4)

#include "IC.h"
#include "gtab.h"
#include "hime-gtk-compatible.h"
#include "im-client/hime-im-client-attr.h"

#if CLIENT_LIB
#define p_err __hime_p_err
#define zmalloc __hime_zmalloc
#endif

#include "util.h"

#define tmalloc(type, n) (type *) malloc (sizeof (type) * (n))
void *zmalloc (int n);
void *memdup (void *p, int n);
#define tzmalloc(type, n) (type *) zmalloc (sizeof (type) * (n))
#define trealloc(p, type, n) (type *) realloc (p, sizeof (type) * (n + 1))
#define tmemdup(p, type, n) (type *) memdup (p, sizeof (type) * n)
extern Display *dpy;

extern char *TableDir;
extern GtkWidget *gwin0;
extern GdkWindow *gdkwin0;
extern Window xwin0;
extern Window root;
void loadIC ();
IC *FindIC (CARD16 icid);
extern ClientState *current_CS;

enum {
    InputStyleOverSpot = 1,
    InputStyleRoot = 2,
    InputStyleOnSpot = 4
};

typedef enum {
    Control_Space = 0,
    Shift_Space = 1,
    Alt_Space = 2,
    Windows_Space = 3,
} IM_TOGGLE_KEYS;

enum {
    TSIN_CHINESE_ENGLISH_TOGGLE_KEY_None = 0,
    TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock = 1,
    TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Tab = 2,
    TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Shift = 4,
    TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftL = 8,
    TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftR = 16,
};

typedef enum {
    TSIN_SPACE_OPT_SELECT_CHAR = 1,
    TSIN_SPACE_OPT_INPUT = 2,
} TSIN_SPACE_OPT;

enum {
    HIME_EDIT_DISPLAY_OVER_THE_SPOT = 1,
    HIME_EDIT_DISPLAY_ON_THE_SPOT = 2,
    HIME_EDIT_DISPLAY_BOTH = 4,
};

#if TRAY_ENABLED
enum {
    HIME_TRAY_DISPLAY_SINGLE = 1,
    HIME_TRAY_DISPLAY_DOUBLE = 2,
    HIME_TRAY_DISPLAY_APPINDICATOR = 3,
};
#endif

#define HIME_SEL_KEY_COLOR_DEFAULT "blue"
#define TSIN_CURSOR_COLOR_DEFAULT "blue"

#define ROW_ROW_SPACING (2)

#define MAX_HIME_STR (256)

#define PHO_KBM "phokbm"

extern int win_xl, win_yl;
extern int win_x, win_y;                        // actual win x/y
extern int current_in_win_x, current_in_win_y;  // request x/y
extern int dpy_xl, dpy_yl;

extern int hime_font_size;

void big5_utf8 (char *s, char out[]);
void utf8_big5 (char *s, char out[]);
gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);

#include "hime-conf.h"

#define bchcpy(a, b) memcpy (a, b, CH_SZ)
#define bchcmp(a, b) memcmp (a, b, CH_SZ)

int utf8_sz (char *s);
int utf8cpy (char *t, char *s);
int u8cpy (char *t, char *s);
int utf8_tlen (char *s, int N);
void utf8_putchar (char *s);
void utf8_putcharn (char *s, int n);
gboolean utf8_eq (char *a, char *b);
gboolean utf8_str_eq (char *a, char *b, int len);
void utf8cpyN (char *t, char *s, int N);
int utf8_str_N (char *str);
void utf8cpyn (char *t, char *s, int n);
void utf8cpy_bytes (char *t, char *s, int n);
char *myfgets (char *buf, int bufN, FILE *fp);
char *half_char_to_full_char (KeySym xkey);
void send_text (char *text);
void send_utf8_ch (char *bchar);
void send_ascii (char key);

// hime.c
void message_cb (char *message);

// hime-common.c
void bell (void);
void case_inverse (KeySym *xkey, int shift_m);
gint64 current_time (void);
void disp_pho_sub (GtkWidget *label, int index, char *pho);
void exec_hime_setup (void);
void set_label_font_size (GtkWidget *label, int size);
void set_label_space (GtkWidget *label);
void set_no_focus (GtkWidget *win);
GdkDisplay *get_default_display (void);
#if GTK_CHECK_VERSION(3, 0, 0)
GdkMonitor *get_primary_monitor (void);
#endif
GdkKeymap *get_keymap (void);
gboolean get_caps_lock_state (void);
Atom get_atom_by_name (Display *display, const char *name);

// hime-settings.c
void load_settings (void);

// pinyin.c
void load_pin_juyin (void);

// im-client/hime-send.c
void send_hime_message (Display *display, const char *message);

// eve.c
gboolean ProcessKeyPress (KeySym keysym, uint32_t kev_state);
gboolean ProcessKeyRelease (KeySym keysym, uint32_t kev_state);
int hime_FocusIn (ClientState *cs);
int hime_FocusOut (ClientState *cs);
int hime_get_preedit (ClientState *cs, char *str, HIME_PREEDIT_ATTR attr[], int *cursor, int *sub_comp_len);
void clear_output_buffer (void);
void flush_edit_buffer (void);
void hide_in_win (ClientState *cs);
void hime_reset (void);
void init_state_chinese (ClientState *cs);
void save_CS_temp_to_current (void);
void update_in_win_pos (void);

// util.c
void dbg_time (char *fmt, ...);

void check_CS ();
void get_win_size (GtkWidget *win, int *width, int *height);
void change_win_fg_bg (GtkWidget *win, GtkWidget *label);
void change_win_bg (GtkWidget *win);
gboolean hime_edit_display_ap_only ();
gboolean hime_display_on_the_spot_key ();
void char_play (char *utf8);
void skip_utf8_sigature (FILE *fp);

#define BITON(flag, bit) ((flag) & (bit))

typedef int usecount_t;

#define MAX_CIN_PHR (100 * CH_SZ + 1)

#endif /* HIME_H */
