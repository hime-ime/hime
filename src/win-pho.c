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

#include "win-common.h"
#include "win-pho.h"
#include "win-sym.h"

static int current_hime_inner_frame;
static int current_pho_in_row1;

GtkWidget *win_pho;
static GtkWidget *top_bin, *hbox_row2;
static GtkWidget *label_pho_sele;
static GtkWidget *label_pho;
static GtkWidget *label_full;
static GtkWidget *label_key_codes;

void create_win_pho (void);
void create_win_pho_gui (void);
void change_pho_font_size ();

void init_win_pho (void) {
    create_win_pho ();
    create_win_pho_gui ();
}

void destroy_win_pho () {
    if (!win_pho)
        return;
    gtk_widget_destroy (win_pho);
    win_pho = NULL;
}

gboolean is_win_pho_visible () {
    return win_pho && gtk_widget_get_visible (win_pho);
}

void set_phoneme_at_index (int index, char *phochar) {
    //  dbg("%d '", index); utf8_putchar(phochar); dbg("'\n");
    set_phoneme_at_index_in_label (label_pho, index, phochar);
}

gboolean win_size_exceed (GtkWidget *win) {
    int width, height;

    get_win_size (win, &width, &height);

    return (width + current_in_win_x > display_width || height + current_in_win_y > display_height);
}

void disp_pho_sel (char *s) {
    if (!label_pho_sele)
        return;
    gtk_label_set_markup (GTK_LABEL (label_pho_sele), s);

    if (win_size_exceed (win_pho)) {
        move_win_pho (current_in_win_x, current_in_win_y);
    }
}

void set_key_codes_label_pho (char *s) {
    if (!label_key_codes)
        return;

    if (!s || !*s) {
        gtk_widget_hide (label_key_codes);
        return;
    }

    gtk_label_set_text (GTK_LABEL (label_key_codes), s);
    gtk_widget_show (label_key_codes);
}

void move_win_pho (int x, int y) {
    move_win (win_pho, x, y);
    move_win_sym ();
}

void create_win_pho () {
    if (win_pho)
        return;

    win_pho = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_has_resize_grip (GTK_WINDOW (win_pho), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (win_pho), 0);
    gtk_widget_realize (win_pho);
    set_no_focus (win_pho);
    apply_widget_bg_color (win_pho);
}

static void mouse_button_callback (GtkWidget *widget, GdkEventButton *event, gpointer data) {
    //  dbg("mouse_button_callback %d\n", event->button);
    switch (event->button) {
    case 1:
        toggle_symbol_table ();
        break;
    case 2:
        inmd_switch_popup_handler (widget, (GdkEvent *) event);
        break;
    case 3:
        exec_hime_setup ();
        break;
    }
}

void create_win_pho_gui_simple () {
    if (top_bin)
        return;

    GtkWidget *vbox_top = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_top), GTK_ORIENTATION_VERTICAL);

    GtkWidget *event_box_pho;
    if (gtab_in_area_button) {
        event_box_pho = gtk_button_new ();
    } else {
        event_box_pho = gtk_event_box_new ();
        gtk_event_box_set_visible_window (GTK_EVENT_BOX (event_box_pho), FALSE);
    }

    gtk_container_set_border_width (GTK_CONTAINER (event_box_pho), 0);

    if (hime_inner_frame) {
        GtkWidget *frame = top_bin = gtk_frame_new (NULL);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
        gtk_container_add (GTK_CONTAINER (win_pho), frame);
        gtk_container_add (GTK_CONTAINER (frame), vbox_top);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    } else {
        gtk_container_add (GTK_CONTAINER (win_pho), vbox_top);
        top_bin = vbox_top;
    }

#if GTK_CHECK_VERSION(3, 0, 0)
#else
    GtkWidget *align = gtk_alignment_new (0, 0, 0, 0);
#endif
    label_pho_sele = gtk_label_new (NULL);

    if (!pho_in_row1) {
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_halign (vbox_top, GTK_ALIGN_START);
        gtk_widget_set_valign (vbox_top, GTK_ALIGN_START);
        gtk_container_add (GTK_CONTAINER (vbox_top), label_pho_sele);
#else
        gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);
        gtk_container_add (GTK_CONTAINER (align), label_pho_sele);
#endif
    } else {
        GtkWidget *hbox_row1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row1, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox_row1), event_box_pho, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_halign (hbox_row1, GTK_ALIGN_START);
        gtk_widget_set_valign (hbox_row1, GTK_ALIGN_START);
        gtk_container_add (GTK_CONTAINER (hbox_row1), label_pho_sele);
#else
        gtk_box_pack_start (GTK_BOX (hbox_row1), align, FALSE, FALSE, 0);
        gtk_container_add (GTK_CONTAINER (align), label_pho_sele);
#endif
    }

    hbox_row2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    /* This packs the button into the win_pho (a gtk container). */
    gtk_container_add (GTK_CONTAINER (vbox_top), hbox_row2);
    label_full = gtk_label_new (_ ("全"));
    gtk_container_add (GTK_CONTAINER (hbox_row2), label_full);

    if (!pho_in_row1)
        gtk_box_pack_start (GTK_BOX (hbox_row2), event_box_pho, FALSE, FALSE, 0);

    g_signal_connect (
        G_OBJECT (event_box_pho), "button-press-event",
        G_CALLBACK (mouse_button_callback), NULL);

    label_pho = gtk_label_new (NULL);

    GtkWidget *frame_pho;
    if (gtab_in_area_button) {
        gtk_container_add (GTK_CONTAINER (event_box_pho), label_pho);
    } else {
        frame_pho = gtk_frame_new (NULL);
        gtk_frame_set_shadow_type (GTK_FRAME (frame_pho), GTK_SHADOW_OUT);
        gtk_container_add (GTK_CONTAINER (event_box_pho), frame_pho);
        gtk_container_set_border_width (GTK_CONTAINER (frame_pho), 0);
        gtk_container_add (GTK_CONTAINER (frame_pho), label_pho);
    }

    label_key_codes = gtk_label_new (NULL);
    gtk_label_set_selectable (GTK_LABEL (label_key_codes), TRUE);
    gtk_box_pack_start (GTK_BOX (hbox_row2), label_key_codes, FALSE, FALSE, 2);

    change_pho_font_size ();
    apply_widget_fg_color (label_pho);
    apply_widget_bg_color (label_pho);

    gtk_widget_show_all (win_pho);

    gtk_widget_hide (label_key_codes);

    gtk_widget_hide (label_full);
}

void create_win_pho_gui () {
    create_win_pho_gui_simple ();

    if (pho_hide_row2) {
        gtk_widget_hide (hbox_row2);
    }

    current_hime_inner_frame = hime_inner_frame;
    current_pho_in_row1 = pho_in_row1;
}

gboolean pho_has_input ();

void show_win_pho () {
    if (hime_pop_up_win && !pho_has_input ())
        return;

    if (!gtk_widget_get_visible (win_pho)) {
        gtk_widget_show (win_pho);
        move_win_pho (win_x, win_y);
    }

    gtk_widget_show (win_pho);
    if (current_CS->b_raise_window)
        gtk_window_present (GTK_WINDOW (win_pho));

    if (pho_hide_row2)
        gtk_widget_hide (hbox_row2);
    else
        gtk_widget_show (hbox_row2);
}

void hide_win_pho () {
    // dbg("hide_win_pho\n");
    if (!win_pho)
        return;

    gtk_widget_hide (win_pho);
}

void init_tab_pho ();
void get_win_gtab_geom ();

void move_gtab_pho_query_win () {
    get_win_gtab_geom ();
    move_win_pho (win_x, win_y + input_window_height);
}

void init_gtab_pho_query_win () {
    init_tab_pho ();
    move_gtab_pho_query_win ();
}

void win_pho_disp_half_full () {
    if (hime_use_custom_theme)
        gtk_label_set_markup (GTK_LABEL (label_pho), get_full_str ());
    else
        gtk_label_set_text (GTK_LABEL (label_pho), get_full_str ());

    if (current_CS->b_im_enabled && !current_fullwidth_mode ())
        gtk_widget_hide (label_full);
    else
        gtk_widget_show (label_full);
}

void get_win_pho_geom () {
    get_win_geom (win_pho);
}

void change_pho_font_size () {
    if (!top_bin)
        return;

    set_label_font_size (label_pho, hime_font_size_tsin_pho_in);

    set_label_font_size (label_pho_sele, hime_font_size);

    change_win_fg_bg (win_pho, label_pho_sele);
}
