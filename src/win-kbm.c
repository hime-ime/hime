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

#include <sys/stat.h>

#include <X11/extensions/XTest.h>

#include "hime.h"

#include "gtab.h"
#include "win-kbm.h"

static GtkWidget *gwin_kbm = NULL;
static int kbm_timeout_handle;

#if !GTK_CHECK_VERSION(2, 91, 6)
static GdkColor red;
#else
static GdkRGBA red;
#endif

gboolean win_kbm_on = FALSE;

enum {
    K_FILL = 1,
    K_HOLD = 2,
    K_PRESS = 4,
    K_AREA_R = 8,
    K_CAPSLOCK = 16
};

typedef struct {
    KeySym keysym;
    unich_t *enkey;
    char shift_key;
    char flag;
    GtkWidget *lab, *but, *laben;
} KEY;

#if TRAY_ENABLED
extern void update_item_active_all ();
#endif

/**
 @brief Virtual keyboard definition

 Some rare users maybe need to translate those key defines.
 So we kept those N_("stuff").

 Note that our po/Makefile do not search .h files so those
 strings will not present (by default) in .pot nor .po files.

*/
#define ROWN 6
#define COLN 19
static KEY keys[ROWN][COLN] = {
    {{XK_Escape, N_ ("Esc")},
     {XK_F1, N_ ("F1")},
     {XK_F2, N_ ("F2")},
     {XK_F3, N_ ("F3")},
     {XK_F4, N_ ("F4")},
     {XK_F5, N_ ("F5")},
     {XK_F6, N_ ("F6")},
     {XK_F7, N_ ("F7")},
     {XK_F8, N_ ("F8")},
     {XK_F9, N_ ("F9")},
     {XK_F10, N_ ("F10")},
     {XK_F11, N_ ("F11")},
     {XK_F12, N_ ("F12")},
     {XK_Print, N_ ("Pr"), 0, K_AREA_R},
     {XK_Scroll_Lock, N_ ("Slk"), 0, K_AREA_R},
     {XK_Pause, N_ ("Pau"), 0, K_AREA_R}},

    {{'`', N_ (" ` "), '~'},
     {'1', N_ (" 1 "), '!'},
     {'2', N_ (" 2 "), '@'},
     {'3', N_ (" 3 "), '#'},
     {'4', N_ (" 4 "), '$'},
     {'5', N_ (" 5 "), '%'},
     {'6', N_ (" 6 "), '^'},
     {'7', N_ (" 7 "), '&'},
     {'8', N_ (" 8 "), '*'},
     {'9', N_ (" 9 "), '('},
     {'0', N_ (" 0 "), ')'},
     {'-', N_ (" - "), '_'},
     {'=', N_ (" = "), '+'},
     {XK_BackSpace, N_ ("←"), 0, K_FILL},
     {XK_Insert, N_ ("Ins"), 0, K_AREA_R},
     {XK_Home, N_ ("Ho"), 0, K_AREA_R},
     {XK_Prior, N_ ("P↑"), 0, K_AREA_R}},

    {{XK_Tab, N_ ("Tab")},
     {'q', N_ (" q ")},
     {'w', N_ (" w ")},
     {'e', N_ (" e ")},
     {'r', N_ (" r ")},
     {'t', N_ (" t ")},
     {'y', N_ (" y ")},
     {'u', N_ (" u ")},
     {'i', N_ (" i ")},
     {'o', N_ (" o ")},
     {'p', N_ (" p ")},
     {'[', N_ (" [ "), '{'},
     {']', N_ (" ] "), '}'},
     {'\\', N_ (" \\ "), '|', K_FILL},
     {XK_Delete, N_ ("Del"), 0, K_AREA_R},
     {XK_End, N_ ("En"), 0, K_AREA_R},
     {XK_Next, N_ ("P↓"), 0, K_AREA_R}},

    {{XK_Caps_Lock, N_ ("Caps"), 0, K_CAPSLOCK},
     {'a', N_ (" a ")},
     {'s', N_ (" s ")},
     {'d', N_ (" d ")},
     {'f', N_ (" f ")},
     {'g', N_ (" g ")},
     {'h', N_ (" h ")},
     {'j', N_ (" j ")},
     {'k', N_ (" k ")},
     {'l', N_ (" l ")},
     {';', N_ (" ; "), ':'},
     {'\'', N_ (" ' "), '"'},
     {XK_Return, N_ (" Enter "), 0, K_FILL},
     {XK_Num_Lock, N_ ("Num"), 0, K_AREA_R},
     {XK_KP_Add, N_ (" + "), 0, K_AREA_R}},

    {{XK_Shift_L, N_ ("  Shift  "), 0, K_HOLD},
     {'z', N_ (" z ")},
     {'x', N_ (" x ")},
     {'c', N_ (" c ")},
     {'v', N_ (" v ")},
     {'b', N_ (" b ")},
     {'n', N_ (" n ")},
     {'m', N_ (" m ")},
     {',', N_ (" , "), '<'},
     {'.', N_ (" . "), '>'},
     {'/', N_ (" / "), '?'},
     {XK_Shift_R, N_ (" Shift"), 0, K_HOLD | K_FILL},
     {XK_KP_Multiply, N_ (" * "), 0, K_AREA_R},
     {XK_Up, N_ ("↑"), 0, K_AREA_R}},

    {{XK_Control_L, N_ ("Ctrl"), 0, K_HOLD},
     {XK_Super_L, N_ ("◆")},
     {XK_Alt_L, N_ ("Alt"), 0, K_HOLD},
     {' ', N_ ("Space"), 0, K_FILL},
     {XK_Alt_R, N_ ("Alt"), 0, K_HOLD},
     {XK_Super_R, N_ ("◆")},
     {XK_Menu, N_ ("■")},
     {XK_Control_R, N_ ("Ctrl"), 0, K_HOLD},
     {XK_Left, N_ ("←"), 0, K_AREA_R},
     {XK_Down, N_ ("↓"), 0, K_AREA_R},
     {XK_Right, N_ ("→"), 0, K_AREA_R}}};

static int keysN = sizeof (keys) / sizeof (keys[0]);

#if !GTK_CHECK_VERSION(3, 0, 0)
static void mod_fg_all (GtkWidget *label, GdkColor *col) {
    if (!label) {
        return;
    }

    gtk_widget_modify_fg (label, GTK_STATE_NORMAL, col);
    gtk_widget_modify_fg (label, GTK_STATE_ACTIVE, col);
    gtk_widget_modify_fg (label, GTK_STATE_SELECTED, col);
    gtk_widget_modify_fg (label, GTK_STATE_PRELIGHT, col);
}
#else
static void mod_fg_all (GtkWidget *label, GdkRGBA *rgbfg) {
    if (!label) {
        return;
    }

    gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, rgbfg);
    gtk_widget_override_color (label, GTK_STATE_FLAG_ACTIVE, rgbfg);
    gtk_widget_override_color (label, GTK_STATE_FLAG_SELECTED, rgbfg);
    gtk_widget_override_color (label, GTK_STATE_FLAG_PRELIGHT, rgbfg);
}
#endif

static void send_fake_key_eve2 (const KeySym key, const gboolean press) {
    const KeyCode kc = XKeysymToKeycode (dpy, key);
    XTestFakeKeyEvent (dpy, kc, press, CurrentTime);
}

static gboolean timeout_repeat (gpointer data) {
    const KeySym k = GPOINTER_TO_INT (data);
    send_fake_key_eve2 (k, TRUE);
    return TRUE;
}

static gboolean timeout_first_time (gpointer data) {
    const KeySym k = GPOINTER_TO_INT (data);
    dbg ("timeout_first_time %c\n", k);
    send_fake_key_eve2 (k, TRUE);
    kbm_timeout_handle = g_timeout_add (50, timeout_repeat, data);
    return FALSE;
}

static void clear_hold (KEY *k) {
    KeySym keysym = k->keysym;
    GtkWidget *laben = k->laben;
    k->flag &= ~K_PRESS;
    mod_fg_all (laben, NULL);
    send_fake_key_eve2 (keysym, FALSE);
}

static gboolean timeout_clear_hold (gpointer data) {
    clear_hold ((KEY *) data);
    return FALSE;
}

static void clear_kbm_timeout_handle (void) {
    if (!kbm_timeout_handle) {
        return;
    }
    g_source_remove (kbm_timeout_handle);
    kbm_timeout_handle = 0;
}

static void cb_button_click (GtkWidget *wid, KEY *k) {
    KeySym keysym = k->keysym;
    GtkWidget *laben = k->laben;

    dbg ("cb_button_click keysym %d\n", keysym);

    if (k->flag & K_HOLD) {
        if (k->flag & K_PRESS) {
            clear_hold (k);
        } else {
            send_fake_key_eve2 (keysym, TRUE);
            k->flag |= K_PRESS;
            mod_fg_all (laben, &red);
            g_timeout_add (10000, timeout_clear_hold, GINT_TO_POINTER (k));
        }
    } else {
        clear_kbm_timeout_handle ();
        kbm_timeout_handle = g_timeout_add (500, timeout_first_time, GINT_TO_POINTER (keysym));
        send_fake_key_eve2 (keysym, TRUE);
    }
}

static void cb_button_release (GtkWidget *wid, KEY *k) {
    dbg ("cb_button_release %d\n", kbm_timeout_handle);
    clear_kbm_timeout_handle ();

    send_fake_key_eve2 (k->keysym, FALSE);

    for (int i = 0; i < keysN; i++) {
        for (int j = 0; keys[i][j].enkey; j++) {
            if (!(keys[i][j].flag & K_PRESS)) {
                continue;
            }
            keys[i][j].flag &= ~K_PRESS;
            send_fake_key_eve2 (keys[i][j].keysym, FALSE);
            mod_fg_all (keys[i][j].laben, NULL);
        }
    }
}

static void create_win_kbm (void) {
#if !GTK_CHECK_VERSION(3, 0, 0)
    gdk_color_parse ("red", &red);
#else
    gdk_rgba_parse (&red, "red");
#endif

    gwin_kbm = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_has_resize_grip (GTK_WINDOW (gwin_kbm), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (gwin_kbm), 0);

    GtkWidget *hbox_top = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (gwin_kbm), hbox_top);

    GtkWidget *vbox_l = gtk_vbox_new (FALSE, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_l), GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start (GTK_BOX (hbox_top), vbox_l, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_l), 0);

    GtkWidget *vbox_r = gtk_vbox_new (FALSE, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_r), GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start (GTK_BOX (hbox_top), vbox_r, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_r), 0);

    for (int i = 0; i < keysN; i++) {
        GtkWidget *hboxl = gtk_hbox_new (FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (hboxl), 0);
        gtk_box_pack_start (GTK_BOX (vbox_l), hboxl, FALSE, FALSE, 0);

        GtkWidget *hboxr = gtk_hbox_new (FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (hboxr), 0);
        gtk_box_pack_start (GTK_BOX (vbox_r), hboxr, FALSE, FALSE, 0);

        KEY *pk = keys[i];
        for (int j = 0; pk[j].enkey; j++) {
            KEY *ppk = &pk[j];
            const char flag = ppk->flag;
            if (!ppk->keysym) {
                continue;
            }
            GtkWidget *but = pk[j].but = gtk_button_new ();
            gtk_container_set_border_width (GTK_CONTAINER (but), 0);

            g_signal_connect (G_OBJECT (but), "pressed", G_CALLBACK (cb_button_click), ppk);
            if (!(ppk->flag & K_HOLD)) {
                g_signal_connect (G_OBJECT (but), "released", G_CALLBACK (cb_button_release), ppk);
            }

            GtkWidget *hbox = (flag & K_AREA_R) ? hboxr : hboxl;

            if (flag & K_FILL) {
                gtk_box_pack_start (GTK_BOX (hbox), but, TRUE, TRUE, 0);
            } else {
                gtk_box_pack_start (GTK_BOX (hbox), but, FALSE, FALSE, 0);
            }

            GtkWidget *v = gtk_vbox_new (FALSE, 0);
            gtk_orientable_set_orientation (GTK_ORIENTABLE (v), GTK_ORIENTATION_VERTICAL);
            gtk_container_set_border_width (GTK_CONTAINER (v), 0);
            gtk_container_add (GTK_CONTAINER (but), v);

            GtkWidget *laben = ppk->laben = gtk_label_new (_ (ppk->enkey));
            set_label_font_size (laben, hime_font_size_win_kbm_en);
            gtk_box_pack_start (GTK_BOX (v), laben, FALSE, FALSE, 0);

            if (0 < i && i < 5) {
                GtkWidget *label = ppk->lab = gtk_label_new ("  ");
                gtk_box_pack_start (GTK_BOX (v), label, FALSE, FALSE, 0);
            }
        }
    }

    gtk_widget_realize (gwin_kbm);
    set_no_focus (gwin_kbm);
}

#if TRAY_ENABLED
extern GtkStatusIcon *tray_icon;
extern GtkStatusIcon *icon_main;

extern gboolean is_exist_tray ();
extern gboolean is_exist_tray_double ();
#endif

static void move_win_kbm (void) {
    int width = 0;
    int height = 0;
    get_win_size (gwin_kbm, &width, &height);

    int ox = 0;
    int oy = 0;

#if TRAY_ENABLED
    GdkRectangle r;
    GtkOrientation ori;

    if (
        (is_exist_tray () && gtk_status_icon_get_geometry (tray_icon, NULL, &r, &ori)) ||
        (is_exist_tray_double () && gtk_status_icon_get_geometry (icon_main, NULL, &r, &ori))) {
        ox = r.x;
        if (ox + width > dpy_xl) {
            ox = dpy_xl - width;
        }

        if (r.y < 100) {
            oy = r.y + r.height;
        } else {
            oy = r.y - height;
        }
    } else
#endif
    {
        ox = dpy_xl - width;
        oy = dpy_yl - height - 16;
    }

    gtk_window_move (GTK_WINDOW (gwin_kbm), ox, oy);
}

void show_win_kbm (void) {
    if (!gwin_kbm) {
        create_win_kbm ();
        update_win_kbm ();
    }

    gtk_widget_show_all (gwin_kbm);
    win_kbm_on = TRUE;

#if TRAY_ENABLED
    update_item_active_all ();
#endif

    move_win_kbm ();
}

#include "pho.h"

static KEY *get_keys_ent (KeySym keysym) {
    const char shift_chars[] = "~!@#$%^&*()_+{}|:\"<>?";
    const char shift_chars_o[] = "`1234567890-=[]\\;',./";

    for (int i = 0; i < keysN; i++) {
        for (int j = 0; j < COLN; j++) {
            char *p = NULL;
            if (keysym >= 'A' && keysym <= 'Z') {
                keysym += 0x20;
            } else if ((p = strchr (shift_chars, keysym))) {
                keysym = shift_chars_o[p - shift_chars];
            }

            if (keys[i][j].keysym != keysym) {
                continue;
            }
            return &keys[i][j];
        }
    }

    return NULL;
}

static void set_kbm_key (const KeySym keysym, char *str) {
    if (!gwin_kbm) {
        return;
    }

    const KEY *p = get_keys_ent (keysym);
    if (!p) {
        return;
    }

    GtkWidget *label = p->lab;
    char *t = (char *) gtk_label_get_text (GTK_LABEL (label));
    char tt[64];

    if (t && strcmp (t, str)) {
        strcat (strcpy (tt, t), str);
        str = tt;
    }

    if (label) {
        gtk_label_set_text (GTK_LABEL (label), str);
        set_label_font_size (label, hime_font_size_win_kbm);
    }
}

static void clear_kbm (void) {
    for (int i = 0; i < keysN; i++) {
        for (int j = 0; j < COLN; j++) {
            GtkWidget *label = keys[i][j].lab;
            if (label) {
                gtk_label_set_text (GTK_LABEL (label), NULL);
            }

            if (keys[i][j].laben) {
                gtk_label_set_text (GTK_LABEL (keys[i][j].laben), _ (keys[i][j].enkey));
            }
        }
    }
}

static void display_shift_keys (void) {
    for (int i = 127; i > 0; i--) {
        const KEY *p = get_keys_ent (i);
        if (p && p->shift_key) {
            char *t = (char *) gtk_label_get_text (GTK_LABEL (p->lab));
            if (t && t[0]) {
                continue;
            }
            char tt[64];
            tt[0] = p->shift_key;
            tt[1] = 0;
            set_kbm_key (i, tt);
        }
    }
}

void update_win_kbm (void) {
    if (!current_CS || !gwin_kbm) {
        return;
    }

    clear_kbm ();

    if (current_CS->im_state != HIME_STATE_CHINESE) {
        if (current_CS->im_state == HIME_STATE_DISABLED) {
            for (int i = 0; i < keysN; i++) {
                for (int j = 0; j < COLN; j++) {
                    char kstr[2];
                    kstr[0] = keys[i][j].shift_key;
                    kstr[1] = 0;

                    if (keys[i][j].laben) {
                        if (kstr[0]) {
                            gtk_label_set_text (GTK_LABEL (keys[i][j].laben), kstr);
                        }
                        set_label_font_size (keys[i][j].laben, hime_font_size_win_kbm_en);
                    }

                    if (keys[i][j].lab) {
                        if (kstr[0]) {
                            gtk_label_set_text (GTK_LABEL (keys[i][j].lab), _ (keys[i][j].enkey));
                        }
                        set_label_font_size (keys[i][j].lab, hime_font_size_win_kbm_en);
                    }
                }
            }
        }
        goto ret;
    }

    switch (current_method_type ()) {
    case method_type_PHO:
    case method_type_TSIN:
        for (int i = 0; i < 128; i++) {
            char tt[64];
            int ttN = 0;

            for (int j = 0; j < 3; j++) {
                const int num = phkbm.phokbm[i][j].num;
                const int typ = phkbm.phokbm[i][j].typ;
                if (!num) {
                    continue;
                }
                ttN += utf8cpy (&tt[ttN], &pho_chars[typ][num * 3]);
            }

            if (!ttN) {
                continue;
            }
            set_kbm_key (i, tt);
        }

        display_shift_keys ();
        break;

    case method_type_MODULE:
        break;

    default:
        if (!cur_inmd || !cur_inmd->DefChars) {
            return;
        }

        for (int loop = 0; loop < 2; loop++) {
            for (int i = 127; i > 0; i--) {
                const char k = cur_inmd->keymap[i];
                if (!k) {
                    continue;
                }

                char *keyname = &cur_inmd->keyname[k * CH_SZ];
                if (!keyname[0]) {
                    continue;
                }

                if (loop == 0 && !(keyname[0] & 0x80)) {
                    continue;
                }

                if (loop == 1) {
                    const KEY *p = get_keys_ent (i);
                    char *t = (char *) gtk_label_get_text (GTK_LABEL (p->lab));
                    if (t && t[0]) {
                        continue;
                    }
                }

                char tt[64];
                tt[0] = 0;
                if (keyname[0] & 128) {
                    utf8cpy (tt, keyname);
                } else {
                    tt[1] = 0;
                    memcpy (tt, keyname, 2);
                    tt[2] = 0;
                }

                set_kbm_key (i, tt);
            }
        }

        display_shift_keys ();

        break;
    }

ret:
    move_win_kbm ();
}

void hide_win_kbm (void) {
    if (!gwin_kbm) {
        return;
    }

    clear_kbm_timeout_handle ();

    win_kbm_on = FALSE;

#if TRAY_ENABLED
    update_item_active_all ();
#endif

    gtk_widget_hide (gwin_kbm);
}

extern gboolean old_capslock_on;

void win_kbm_disp_caplock () {
    const KEY *p = get_keys_ent (XK_Caps_Lock);

    if (old_capslock_on) {
        mod_fg_all (p->laben, &red);
    } else {
        mod_fg_all (p->laben, NULL);
    }
}
