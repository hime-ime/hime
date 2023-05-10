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

#include "hime.h"

#include "gst.h"
#include "gtab.h"
#include "win-common.h"
#include "win-gtab.h"
#include "win-sym.h"

static int current_hime_inner_frame;
static int current_gtab_in_row1;
static int current_gtab_vertical_select;

GtkWidget *gwin_gtab;
static GtkWidget *top_bin;
static GtkWidget *label_full, *label_gtab_sele, *label_gtab_pre_sel;
static GtkWidget *label_gtab = NULL;
static GtkWidget *label_input_method_name;
static GtkWidget *label_key_codes;
char str_key_codes[128];
gboolean better_key_codes;
static GtkWidget *box_gtab_im_name;
static GtkWidget *hbox_row2;
static GtkWidget *label_page;
static GtkWidget *label_edit;
static GdkRGBA better_color;
gboolean last_cursor_off;

gboolean win_size_exceed (GtkWidget *win), gtab_phrase_on ();
void move_win_gtab (int x, int y);
int win_gtab_max_key_press;

void move_gtab_pho_query_win ();

gboolean is_win_gtab_visible (void) {
    return gwin_gtab && gtk_widget_get_visible (gwin_gtab);
}

static void adj_gtab_win_pos () {
    if (!gwin_gtab)
        return;
    if (win_size_exceed (gwin_gtab))
        move_win_gtab (current_in_win_x, current_in_win_y);
}

void win_gtab_disp_half_full ();

void disp_gtab (char *str) {
    if (!label_gtab)
        return;
    if (str && (str[0] != '\0')) {
        gtk_widget_show (label_gtab);
        gtk_label_set_text (GTK_LABEL (label_gtab), str);
    } else {
        if (hime_status_tray || (!gtab_hide_row2))
            gtk_widget_hide (label_gtab);
        else
            win_gtab_disp_half_full ();
    }

    adj_gtab_win_pos ();
}

void set_gtab_input_color (GdkRGBA *rgbfg) {
    if (label_gtab)
        gtk_widget_override_color (label_gtab, GTK_STATE_FLAG_NORMAL, rgbfg);
}

void set_gtab_input_error_color () {
    GdkRGBA red;
    gdk_rgba_parse (&red, "red");
    set_gtab_input_color (&red);
}

void clear_gtab_input_error_color () {
    if (hime_use_custom_theme) {
        GdkRGBA color;
        gdk_rgba_parse (&color, hime_win_color_fg);
        set_gtab_input_color (&color);
    } else {
        set_gtab_input_color (NULL);
    }
}

static gboolean need_label_edit ();

void gtab_disp_empty (char *tt, int N) {
    int i;

    if (!need_label_edit ())
        return;

    for (i = 0; i < N; i++)
        //    strcat(tt, "﹍");
        strcat (tt, "　"); /* Full width space */
}

void clear_gtab_in_area () {
    if (!cur_inmd)
        return;
    char tt[64];
    tt[0] = 0;
    gtab_disp_empty (tt, win_gtab_max_key_press);
    disp_gtab (tt);
}

static void set_disp_im_name ();

void change_win_fg_bg (GtkWidget *win, GtkWidget *label) {
    apply_widget_bg_color (win);

    apply_widget_fg_color (label);
    apply_widget_fg_color (label_edit);
    apply_widget_fg_color (label_gtab_pre_sel);
    apply_widget_fg_color (label_input_method_name);
    apply_widget_fg_color (label_gtab);
}

void change_gtab_font_size () {
    if (!label_gtab_sele)
        return;

    set_label_font_size (label_gtab_sele, hime_font_size);
    set_label_font_size (label_gtab_pre_sel, hime_font_size_tsin_presel);

    set_label_font_size (label_edit, hime_font_size);

    if (GTK_IS_WIDGET (label_gtab))
        set_label_font_size (label_gtab, hime_font_size_gtab_in);

    set_disp_im_name ();

    change_win_fg_bg (gwin_gtab, label_gtab_sele);
}

void show_win_gtab ();

void disp_gtab_sel (char *s) {
    //  dbg("disp_gtab_sel '%s' %x\n", s, label_gtab_sele);

    if (!label_gtab_sele) {
        if (s && *s)
            show_win_gtab ();
        else
            return;
    }

    if (s && (!s[0]) && (hime_edit_display == HIME_EDIT_DISPLAY_ON_THE_SPOT) && gtab_hide_row2 && (hime_on_the_spot_key || (!gtab_in_row1)))
        gtk_widget_hide (gwin_gtab);
    else {
        if (gwin_gtab && !gtk_widget_get_visible (gwin_gtab))
            show_win_gtab ();
        gtk_widget_show (label_gtab_sele);
    }

    //  dbg("disp_gtab_sel '%s'\n", s);
    gtk_label_set_markup (GTK_LABEL (label_gtab_sele), s);
    adj_gtab_win_pos ();
}

void set_key_codes_label (char *s, int better) {
    if (!label_key_codes) {
        return;
    }

    if (s && strlen (s) && hbox_row2) {
        if (
            !gtab_hide_row2 ||
            ggg.wild_mode ||
            str_key_codes[0]) {
            gtk_widget_show (hbox_row2);
        }
    } else {
        if (gtab_hide_row2) {
            gtk_widget_hide (hbox_row2);
        }
    }

    if (better) {
        gtk_widget_override_color (label_key_codes, GTK_STATE_FLAG_NORMAL, &better_color);
    } else {
        gtk_widget_override_color (label_key_codes, GTK_STATE_FLAG_NORMAL, NULL);
    }

    gtk_label_set_text (GTK_LABEL (label_key_codes), s);

    if (s && s[0])
        gtk_widget_show (label_key_codes);

    better_key_codes = better;
    if (s && s != str_key_codes)
        strcpy (str_key_codes, s);
    else
        str_key_codes[0] = 0;
}

void set_page_label (char *s) {
    if (!label_page)
        return;
    gtk_label_set_text (GTK_LABEL (label_page), s);
    gtk_widget_show (label_page);
}

void move_win_gtab (int x, int y) {
    if (!gwin_gtab)
        return;
    //  dbg("move_win_gtab %d %d\n", x, y);
    get_win_size (gwin_gtab, &win_xl, &win_yl);

    if (x + win_xl > dpy_xl)
        x = dpy_xl - win_xl;
    if (x < 0)
        x = 0;

    if (y + win_yl > dpy_yl)
        y = dpy_yl - win_yl;
    if (y < 0)
        y = 0;

    gtk_window_move (GTK_WINDOW (gwin_gtab), x, y);
    win_x = x;
    win_y = y;

    move_win_sym ();
    if (poo.same_pho_query_state != SAME_PHO_QUERY_none)
        move_gtab_pho_query_win ();
}

void set_gtab_input_method_name (char *s) {
    //  dbg("set_gtab_input_method_name '%s'\n", s);
    if (!gtab_disp_im_name)
        return;
    if (!label_input_method_name)
        return;
    //  dbg("set_gtab_input_method_name b '%s'\n", s);
    gtk_widget_show (label_input_method_name);
    gtk_label_set_text (GTK_LABEL (label_input_method_name), s);
}

gboolean use_tsin_sel_win ();
void init_tsin_selection_win ();

void create_win_gtab () {
    if (gwin_gtab)
        return;

    gwin_gtab = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_has_resize_grip (GTK_WINDOW (gwin_gtab), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (gwin_gtab), 0);
    gtk_widget_realize (gwin_gtab);

    set_no_focus (gwin_gtab);

    if (use_tsin_sel_win ())
        init_tsin_selection_win ();
}

static void mouse_button_callback (GtkWidget *widget, GdkEventButton *event, gpointer data) {
    //  dbg("mouse_button_callback %d\n", event->button);
    switch (event->button) {
    case 1:
        toggle_win_sym ();
        break;
    case 2:
        inmd_switch_popup_handler (widget, (GdkEvent *) event);
        break;
    case 3:
        exec_hime_setup ();
        break;
    }
}

void toggle_half_full_char ();

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);

void show_hide_label_edit () {
    if (!label_edit)
        return;

    if (hime_edit_display_ap_only () || !gtab_phrase_on ()) {
        gtk_widget_hide (label_edit);
    } else
        gtk_widget_show (label_edit);
}

void disp_label_edit (char *str) {
    if (!label_edit)
        return;

    show_hide_label_edit ();

    gtk_label_set_markup (GTK_LABEL (label_edit), str);
}

static gboolean need_label_edit () {
    return gtab_phrase_on () && !hime_edit_display_ap_only ();
}

static int current_gtab_phrase_pre_select, current_hime_on_the_spot_key, current_gtab_disp_im_name;
gboolean gtab_vertical_select_on ();

static void destroy_if_necessary () {
    gboolean new_need_label_edit = need_label_edit ();
    gboolean new_last_cursor_off = gtab_in_row1 && new_need_label_edit;

    //  dbg("zzz %d %d\n", gtab_in_row1, new_need_label_edit);

    if (!top_bin ||
        (current_hime_inner_frame == hime_inner_frame &&
         current_gtab_in_row1 == gtab_in_row1 &&
         new_last_cursor_off == last_cursor_off &&
         current_gtab_vertical_select == gtab_vertical_select_on () &&
         current_gtab_phrase_pre_select == gtab_phrase_pre_select &&
         current_hime_on_the_spot_key == hime_on_the_spot_key &&
         current_gtab_disp_im_name == gtab_disp_im_name &&
         ((new_need_label_edit && label_edit) || (!new_need_label_edit && !label_edit))))
        return;
#if 0
  dbg("hime_inner_frame %d,%d,  gtab_in_row1 %d,%d  cursor_off% d,%d   vert:%d,%d  edit:%d,%d\n",
    current_hime_inner_frame,hime_inner_frame, current_gtab_in_row1,gtab_in_row1,
    new_last_cursor_off, last_cursor_off, current_gtab_vertical_select, gtab_vertical_select_on(),
    new_need_label_edit, label_edit!=0);
#endif
    current_gtab_phrase_pre_select = gtab_phrase_pre_select;
    current_hime_on_the_spot_key = hime_on_the_spot_key;
    current_gtab_disp_im_name = gtab_disp_im_name;

    gtk_widget_destroy (top_bin);
    top_bin = NULL;
    label_edit = NULL;
    hbox_row2 = NULL;
}

void mod_bg_all (GtkWidget *lab, GdkRGBA *col);

void create_win_gtab_gui_simple () {
    destroy_if_necessary ();

    if (top_bin)
        return;

    last_cursor_off = FALSE;

    GtkWidget *vbox_top = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_top), GTK_ORIENTATION_VERTICAL);

    GtkWidget *event_box_gtab;
    if (gtab_in_area_button) {
        event_box_gtab = gtk_button_new ();
    } else {
        event_box_gtab = gtk_event_box_new ();
        gtk_event_box_set_visible_window (GTK_EVENT_BOX (event_box_gtab), FALSE);
    }

    gtk_container_set_border_width (GTK_CONTAINER (event_box_gtab), 0);

    if (gwin_gtab == NULL)
        create_win_gtab ();

    if (hime_inner_frame) {
        GtkWidget *frame = top_bin = gtk_frame_new (NULL);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
        gtk_container_add (GTK_CONTAINER (gwin_gtab), frame);
        gtk_container_set_border_width (GTK_CONTAINER (gwin_gtab), 0);
        gtk_container_add (GTK_CONTAINER (frame), vbox_top);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    } else {
        gtk_container_add (GTK_CONTAINER (gwin_gtab), vbox_top);
        top_bin = vbox_top;
    }

    GtkWidget *hbox_edit = NULL;

    gboolean b_need_label_edit = need_label_edit ();

    if (b_need_label_edit) {
        label_edit = gtk_label_new (NULL);

        hbox_edit = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start (GTK_BOX (vbox_top), hbox_edit, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_halign (hbox_edit, GTK_ALIGN_START);
        gtk_widget_set_valign (hbox_edit, GTK_ALIGN_START);
        gtk_container_add (GTK_CONTAINER (hbox_edit), label_edit);
#else
        GtkWidget *align_edit = gtk_alignment_new (0, 0.0, 0, 0);
        gtk_box_pack_start (GTK_BOX (hbox_edit), align_edit, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (align_edit), label_edit, FALSE, FALSE, 0);
#endif
    }

    label_gtab_sele = gtk_label_new (NULL);

#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_set_halign (vbox_top, GTK_ALIGN_START);
    gtk_widget_set_valign (vbox_top, GTK_ALIGN_START);
#else
    GtkWidget *align = gtk_alignment_new (0, 0.0, 0, 0);
    gtk_container_add (GTK_CONTAINER (align), label_gtab_sele);
#endif

    if (!gtab_in_row1) {
        if (!gtab_vertical_select_on ()) {
#if GTK_CHECK_VERSION(3, 0, 0)
            gtk_container_add (GTK_CONTAINER (vbox_top), label_gtab_sele);
#else
            gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);
#endif
        }
    } else {
        GtkWidget *hbox_row1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row1, FALSE, FALSE, 0);

        if (b_need_label_edit) {
            last_cursor_off = TRUE;
            gtk_box_pack_start (GTK_BOX (hbox_edit), event_box_gtab, FALSE, FALSE, 0);
        } else {
            gtk_box_pack_start (GTK_BOX (hbox_row1), event_box_gtab, FALSE, FALSE, 0);
        }

        if (!gtab_vertical_select_on ()) {
#if GTK_CHECK_VERSION(3, 0, 0)
            gtk_widget_set_halign (hbox_row1, GTK_ALIGN_START);
            gtk_widget_set_valign (hbox_row1, GTK_ALIGN_START);
            gtk_container_add (GTK_CONTAINER (hbox_row1), label_gtab_sele);
#else
            gtk_box_pack_start (GTK_BOX (hbox_row1), align, FALSE, FALSE, 0);
#endif
        }
    }

    if (gtab_phrase_pre_select && !use_tsin_sel_win ()) {
        label_gtab_pre_sel = gtk_label_new (NULL);
        set_label_font_size (label_gtab_pre_sel, hime_font_size_tsin_presel);
        gtk_box_pack_start (GTK_BOX (vbox_top), label_gtab_pre_sel, FALSE, FALSE, 0);
    }

    hbox_row2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row2, FALSE, FALSE, 0);

    label_full = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label_full), _ (cht_full_str));

    gtk_box_pack_start (GTK_BOX (hbox_row2), label_full, FALSE, FALSE, 0);

    if (gtab_disp_im_name) {
        GtkWidget *event_box_input_method_name;
        if (gtab_in_area_button)
            event_box_input_method_name = gtk_button_new ();
        else {
            event_box_input_method_name = gtk_event_box_new ();
            gtk_event_box_set_visible_window (GTK_EVENT_BOX (event_box_input_method_name), FALSE);
        }

        gtk_box_pack_start (GTK_BOX (hbox_row2), event_box_input_method_name, FALSE, FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (event_box_input_method_name), 0);

        GtkWidget *frame_input_method_name = NULL;
        if (!gtab_in_area_button) {
            frame_input_method_name = gtk_frame_new (NULL);
            gtk_frame_set_shadow_type (GTK_FRAME (frame_input_method_name), GTK_SHADOW_OUT);
            gtk_container_add (GTK_CONTAINER (event_box_input_method_name), frame_input_method_name);
            gtk_container_set_border_width (GTK_CONTAINER (frame_input_method_name), 0);
        }

        label_input_method_name = gtk_label_new ("");

        gtk_container_add (
            GTK_CONTAINER (gtab_in_area_button ? event_box_input_method_name : frame_input_method_name),
            label_input_method_name);
        g_signal_connect_swapped (GTK_OBJECT (event_box_input_method_name), "button-press-event",
                                  G_CALLBACK (inmd_switch_popup_handler), NULL);

        box_gtab_im_name = event_box_input_method_name;
    }

    if (!gtab_in_row1)
        gtk_box_pack_start (GTK_BOX (hbox_row2), event_box_gtab, FALSE, FALSE, 0);

    if (!hime_display_on_the_spot_key ()) {
        GtkWidget *frame_gtab = NULL;
        if (!gtab_in_area_button) {
            frame_gtab = gtk_frame_new (NULL);
            gtk_frame_set_shadow_type (GTK_FRAME (frame_gtab), GTK_SHADOW_OUT);
            gtk_container_set_border_width (GTK_CONTAINER (frame_gtab), 0);
            gtk_container_add (GTK_CONTAINER (event_box_gtab), frame_gtab);
        }
        g_signal_connect (
            G_OBJECT (event_box_gtab), "button-press-event",
            G_CALLBACK (mouse_button_callback), NULL);

        label_gtab = gtk_label_new (NULL);

        if (gtab_in_area_button)
            gtk_container_add (GTK_CONTAINER (event_box_gtab), label_gtab);
        else
            gtk_container_add (GTK_CONTAINER (frame_gtab), label_gtab);
    }

    label_key_codes = gtk_label_new (NULL);
    gtk_box_pack_start (GTK_BOX (hbox_row2), label_key_codes, FALSE, FALSE, 2);

    label_page = gtk_label_new (NULL);
    gtk_box_pack_start (GTK_BOX (hbox_row2), label_page, FALSE, FALSE, 2);

    if (gtab_vertical_select_on ()) {
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_halign (vbox_top, GTK_ALIGN_START);
        gtk_widget_set_valign (vbox_top, GTK_ALIGN_START);
        gtk_container_add (GTK_CONTAINER (vbox_top), label_gtab_sele);
#else
        gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);
#endif
    }

    change_gtab_font_size ();

    if (
        (current_method_type () != method_type_PHO) &&
        (current_method_type () != method_type_PHO)) {
        gtk_widget_show_all (gwin_gtab);
    }
    gtk_widget_hide (gwin_gtab);
    gtk_widget_hide (label_gtab_sele);
    gtk_widget_hide (label_key_codes);
    gtk_widget_hide (label_page);

    if (GTK_IS_WIDGET (label_gtab_pre_sel))
        gtk_widget_hide (label_gtab_pre_sel);

    show_hide_label_edit ();

    set_disp_im_name ();
    gtk_widget_hide (label_full);

    if (gtab_hide_row2)
        gtk_widget_hide (hbox_row2);
}

void disp_gtab_pre_sel (char *s) {
    //  dbg("disp_gtab_pre_sel %s\n", s);
    if (!label_gtab_pre_sel)
        show_win_gtab ();

    //  dbg("label_gtab_pre_sel %x %d\n", label_gtab_pre_sel, use_tsin_sel_win());
    gtk_widget_show (label_gtab_pre_sel);
    if (s)
        gtk_label_set_markup (GTK_LABEL (label_gtab_pre_sel), s);
    show_win_gtab ();
    adj_gtab_win_pos ();
}

void show_input_method_name (GtkWidget *label, gchar *cname) {
    if (hime_use_custom_theme) {
        gchar *color_cname = g_strdup_printf ("<span foreground=\"%s\">[%s]</span>",
                                              hime_sel_key_color, cname);
        gtk_label_set_markup (GTK_LABEL (label), color_cname);
        g_free (color_cname);
    } else
        gtk_label_set_text (GTK_LABEL (label), cname);
}

void show_input_method_name_on_gtab () {
    if (current_CS && (!hime_status_tray) && gtab_disp_im_name &&
        current_CS->b_im_enabled && !current_fullwidth_mode () &&
        (chinese_mode ())) {
        if ((current_method_type () == method_type_MODULE) ||
            (gtab_hide_row2 && (hime_edit_display == HIME_EDIT_DISPLAY_ON_THE_SPOT)))
        // label_gtab = NULL under onthespot mode.
        {
            if (current_method_type () == method_type_MODULE) {
                show_win_gtab ();
                adj_gtab_win_pos ();
                if (hbox_row2)
                    gtk_widget_hide (hbox_row2);
            }

            if (label_gtab == NULL)
                create_win_gtab_gui_simple ();
            disp_gtab_sel (NULL);
            show_input_method_name (label_gtab_sele, inmd[current_CS->in_method].cname);
            return;
        }

        if ((label_gtab == NULL) || (!gtab_hide_row2))
            return;

        show_input_method_name (label_gtab, inmd[current_CS->in_method].cname);
    }
}

static void create_win_gtab_gui () {
    create_win_gtab_gui_simple ();
    current_gtab_in_row1 = gtab_in_row1;
    current_gtab_vertical_select = gtab_vertical_select_on ();
    current_hime_inner_frame = hime_inner_frame;
    gdk_rgba_parse (&better_color, "red");
}

void change_win_gtab_style () {
    destroy_if_necessary ();
    create_win_gtab_gui ();
}

void init_gtab (int inmdno);
gboolean gtab_has_input ();
extern gboolean force_show;

void show_win_gtab () {
    create_win_gtab ();
    create_win_gtab_gui ();
    // window was destroyed
    if (hime_pop_up_win)
        set_key_codes_label (str_key_codes, better_key_codes);

    if (current_CS) {
        if (current_CS->fixed_pos)
            move_win_gtab (0, 0);
    }

    //  init_gtab(current_CS->in_method);

    if (hime_pop_up_win && !gtab_has_input () &&
        !force_show && poo.same_pho_query_state == SAME_PHO_QUERY_none && !tss.pre_selN)
        return;

        //  dbg("show_win_gtab()\n");

#if 0
  if (current_CS->b_raise_window)
#endif
    gtk_window_present (GTK_WINDOW (gwin_gtab));

    move_win_gtab (current_in_win_x, current_in_win_y);

    if ((current_method_type () != method_type_PHO) && (current_method_type () != method_type_PHO))
        gtk_widget_show (gwin_gtab);

    if (current_CS) {
        if (!chinese_mode ())
            set_gtab_input_method_name (eng_half_str);
        else
            set_gtab_input_method_name (inmd[current_CS->in_method].cname);

        if ((GTK_IS_WIDGET (label_gtab)) && (hime_status_tray || (!gtab_hide_row2)))
            gtk_widget_hide (label_gtab);

        win_gtab_disp_half_full ();
    }

    show_win_sym ();
}

void close_gtab_pho_win ();

static void destroy_top_bin () {
    if (!top_bin)
        return;
    gtk_widget_destroy (top_bin);
    top_bin = NULL;
    hbox_row2 = NULL;
    label_full = NULL;
    label_gtab_sele = NULL;
    label_gtab = NULL;
    label_input_method_name = NULL;
    label_key_codes = NULL;
    box_gtab_im_name = NULL;
    label_page = NULL;
    label_edit = NULL;
    label_gtab_pre_sel = NULL;
}

void destroy_win_gtab () {
    if (!gwin_gtab)
        return;
    destroy_top_bin ();
    gtk_widget_destroy (gwin_gtab);
    gwin_gtab = NULL;
}

void hide_win_kbm ();

void hide_win_gtab () {
    win_gtab_max_key_press = 0;

    if (!gwin_gtab)
        return;

    //  dbg("hide_win_gtab\n");
    if (gwin_gtab) {
        gtk_widget_hide (gwin_gtab);
        destroy_top_bin ();
    }

    close_gtab_pho_win ();
    hide_win_sym ();
    hide_win_kbm ();
}

void get_win_gtab_geom () {
    get_win_geom (gwin_gtab);
}

static void set_disp_im_name () {
    if (!box_gtab_im_name)
        return;

    if (gtab_disp_im_name)
        gtk_widget_show (box_gtab_im_name);
    else
        gtk_widget_hide (box_gtab_im_name);
}

void win_gtab_disp_half_full () {
    if (!gwin_gtab)
        return;
    if (label_full) {
        if ((current_CS->b_im_enabled && !current_fullwidth_mode ()) ||
            (!chinese_mode ()))
            gtk_widget_hide (label_full);
        else
            gtk_widget_show (label_full);
    }

    if (chinese_mode ()) {
        if (label_gtab_sele)
            gtk_widget_show (label_gtab_sele);
        if (hime_status_tray || (!gtab_hide_row2))
            if (GTK_IS_WIDGET (label_gtab))
                gtk_widget_show (label_gtab);
    } else {
        if (label_gtab_sele)
            gtk_widget_hide (label_gtab_sele);
        if (hime_status_tray || (!gtab_hide_row2))
            if (label_gtab)
                gtk_widget_hide (label_gtab);
    }

    if (gtab_hide_row2) {
        if (GTK_IS_WIDGET (label_gtab)) {
            if (hime_use_custom_theme)
                gtk_label_set_markup (GTK_LABEL (label_gtab), get_full_str ());
            else
                gtk_label_set_text (GTK_LABEL (label_gtab), get_full_str ());
        } else {
            gchar *full_str = get_full_str ();
            if (full_str && full_str[0])
                disp_gtab_sel (full_str);
            else
                disp_gtab_sel (inmd[current_CS->in_method].cname);
        }
    }
}

void hide_selections_win ();

void hide_gtab_pre_sel () {
    if (use_tsin_sel_win ())
        hide_selections_win ();

    //  dbg("hide_gtab_pre_sel %d\n", tss.ctrl_pre_sel);
    tss.pre_selN = 0;
    tss.ctrl_pre_sel = FALSE;
    if (label_gtab_pre_sel)
        gtk_widget_hide (label_gtab_pre_sel);

    move_win_gtab (current_in_win_x, current_in_win_y);
    adj_gtab_win_pos ();
}
