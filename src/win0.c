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
#include "pho.h"
#include "tsin.h"
#include "win-common.h"
#include "win-sym.h"
#include "win0.h"

GtkWidget *win0 = NULL;
static GtkWidget *top_bin = NULL;
static GtkWidget *hbox_edit = NULL;
static GtkWidget *button_pho = NULL;
static GtkWidget *label_pho = NULL;

int current_hime_inner_frame;

static PangoAttrList *attr_list, *attr_list_blank;

static struct {
    GtkWidget *vbox;
    GtkWidget *label;
    int x;
} chars[MAX_PH_BF_EXT];

extern int text_pho_N;
extern gboolean force_show;

int get_widget_xy (GtkWidget *win, GtkWidget *widget, int *rx, int *ry);
static void set_win0_bg ();
static void create_cursor_attr ();
static void mouse_button_callback (GtkWidget *widget, GdkEventButton *event, gpointer data);
static void mouse_char_callback (GtkWidget *widget, GdkEventButton *event, gpointer data);

// External function
void show_selections_win ();
void hide_selections_win ();
void init_tsin_selection_win ();
void getRootXY (Window win, int wx, int wy, int *tx, int *ty);
void disp_selections (int x, int y);
void open_select_pho ();
void create_phrase_save_menu (GdkEventButton *event);

static void recreate_win0 () {
    destroy_win0 ();
    init_win0 ();
}

void init_win0 () {
    if (win0)
        return;
    win0 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_has_resize_grip (GTK_WINDOW (win0), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (win0), 0);
    gtk_widget_realize (win0);
    set_no_focus (win0);

    GtkWidget *vbox_top = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_top), GTK_ORIENTATION_VERTICAL);
    gtk_container_set_border_width (GTK_CONTAINER (win0), 0);

    if (hime_inner_frame) {
        GtkWidget *frame;
        top_bin = frame = gtk_frame_new (NULL);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
        gtk_container_add (GTK_CONTAINER (win0), frame);
        gtk_container_add (GTK_CONTAINER (frame), vbox_top);
    } else {
        top_bin = vbox_top;
        gtk_container_add (GTK_CONTAINER (win0), vbox_top);
    }

    reset_content ();

    GtkWidget *hbox_row1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    /* This packs the button into the win0 (a gtk container). */
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row1, FALSE, FALSE, 0);

    hbox_edit = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox_edit), 0);
    /* This packs the button into the win0 (a gtk container). */
    gtk_box_pack_start (GTK_BOX (hbox_row1), hbox_edit, FALSE, FALSE, 0);

    create_cursor_attr ();

    button_pho = gtk_button_new ();
    gtk_container_set_border_width (GTK_CONTAINER (button_pho), 0);
    gtk_box_pack_start (GTK_BOX (hbox_row1), button_pho, FALSE, FALSE, 0);

    g_signal_connect (G_OBJECT (button_pho), "button-press-event",
                      G_CALLBACK (mouse_button_callback), NULL);
    gtk_widget_set_can_focus (button_pho, FALSE);
    gtk_widget_set_can_default (button_pho, FALSE);

    label_pho = gtk_label_new ("");
    set_label_font_size (label_pho, hime_font_size_tsin_pho_in);
    gtk_container_add (GTK_CONTAINER (button_pho), label_pho);

    clear_phonemes ();

    gtk_widget_show_all (win0);
    //  gdk_flush();
    gtk_widget_hide (win0);

    init_tsin_selection_win ();

    set_win0_bg ();
}

void destroy_win0 () {
    if (!win0)
        return;
    gtk_widget_destroy (win0);
    win0 = NULL;
    top_bin = NULL;
    label_pho = NULL;
    button_pho = NULL;
    hbox_edit = NULL;
    reset_content ();
}

static void create_char_widgets (int index) {

    if (!hbox_edit)
        return;

    GdkRGBA color_bg;
    gdk_rgba_parse (&color_bg, tsin_phrase_line_color);

    if (chars[index].vbox)
        return;

    GtkWidget *event_box = gtk_event_box_new ();
    gtk_event_box_set_visible_window (GTK_EVENT_BOX (event_box), FALSE);
    chars[index].vbox = event_box;
    g_signal_connect (
        G_OBJECT (event_box), "button-press-event",
        G_CALLBACK (mouse_char_callback), GINT_TO_POINTER (index));

    gtk_box_pack_start (GTK_BOX (hbox_edit), event_box, FALSE, FALSE, 0);
    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox), GTK_ORIENTATION_VERTICAL);
    gtk_container_add (GTK_CONTAINER (event_box), vbox);

    GtkWidget *label = gtk_label_new (NULL);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

    set_label_font_size (label, hime_font_size);
    chars[index].label = label;

    apply_widget_fg_color (label);
    gtk_widget_show_all (event_box);
}

void show_win0 () {
    if (hime_pop_up_win && !tsin_has_input () && !force_show) {
        //    dbg("show ret\n");
        return;
    }

#if 0
  if (!gtk_widget_get_visible(win0))
#endif
    {
        //    dbg("gtk_widget_show %x\n", win0);
        move_win0 (win_x, win_y);
        gtk_widget_show (win0);
    }

    if (current_CS->b_raise_window) {
        gtk_window_present (GTK_WINDOW (win0));
        show_selections_win ();
    }
}

void hide_win0 () {
    if (!win0)
        return;

    gtk_widget_hide (win0);

    hide_selections_win ();
}

void disp_tsin_select (int index) {
    int x, y;

    if (index < 0)
        return;

    //  dbg("hime_edit_display_ap_only() %d\n", hime_edit_display_ap_only());

    if (hime_edit_display_ap_only ()) {
        getRootXY (current_CS->client_win, current_CS->spot_location.x, current_CS->spot_location.y, &x, &y);
    } else {
#if 1
        int i;
        // bug in GTK, widget position is wrong, repeat util find one
        for (i = index; i >= 0; i--) {
            gtk_widget_show_now (chars[i].label);
            gtk_widget_show (chars[i].vbox);
            gtk_main_iteration_do (FALSE);

            int tx = get_widget_xy (win0, chars[i].vbox, &x, &y);

            if (tx >= 0)
                break;
        }
#else
        get_widget_xy (win0, chars[index].vbox, &x, &y);
#endif
        get_win0_geom ();
    }
    disp_selections (x, y);
}

void set_and_show_char (int index, char *ch) {
    if (hime_edit_display_ap_only ())
        return;
    if (!win0)
        show_win0 ();

    create_char_widgets (index);
    GtkWidget *label = chars[index].label;

    if (label) {
        if (ch[0] == ' ' && ch[1] == ' ')
            set_label_space (label);
        else {
            gtk_label_set_text (GTK_LABEL (label), ch);
        }
    }

    get_win0_geom ();
    move_win0 (win_x, win_y);

    gtk_widget_show_all (chars[index].vbox);
}

void clear_and_hide_chars_all () {
    int i;
    for (i = 0; i < MAX_PH_BF_EXT; i++) {
        clear_and_hide_char (i);
    }

    move_win0_auto ();
}

void clear_and_hide_char (int index) {
    if (!chars[index].vbox)
        return;
    gtk_label_set_text (GTK_LABEL (chars[index].label), "");
    gtk_widget_hide (chars[index].vbox);
}

void set_cursor (int index) {
    GtkWidget *label = chars[index].label;

    if (!label)
        return;

    if (hime_edit_display_ap_only ())
        return;

    gtk_label_set_attributes (GTK_LABEL (label), attr_list);
}

void clear_cursor (int index) {
    GtkWidget *label = chars[index].label;

    if (!label)
        return;
    gtk_label_set_attributes (GTK_LABEL (label), attr_list_blank);
}

void show_button_pho () {
    if (!button_pho)
        return;

    gtk_widget_show (button_pho);
}

void hide_button_pho () {
    if (!button_pho)
        return;

    gtk_widget_hide (button_pho);
    move_win0_auto ();
}

gboolean is_win0_visible (void) {
    return win0 && gtk_widget_get_visible (win0);
}

void get_win0_geom () {
    get_win_geom (win0);
}

int get_widget_xy (GtkWidget *win, GtkWidget *widget, int *rx, int *ry) {
    if (!win && !widget)
        p_err ("get_widget_xy err");

    //  gdk_flush();

    GtkRequisition sz;
    gtk_widget_get_preferred_size (widget, NULL, &sz);
    int wx, wy;

    wx = wy = 0;

    gtk_widget_translate_coordinates (widget, win,
                                      0, sz.height, &wx, &wy);

    gtk_widget_translate_coordinates (widget, win,
                                      0, sz.height, &wx, &wy);

    //  dbg("%d wx:%d\n", index,  wx);

    int win_x, win_y;

    gtk_window_get_position (GTK_WINDOW (win), &win_x, &win_y);
    int input_window_width, input_window_height;
    get_win_size (win, &input_window_width, &input_window_height);

    if (wx > input_window_width)
        wx = input_window_width;

    *rx = win_x + wx;
    *ry = win_y + wy;
    return wx;
}

void move_win0_auto () {
    if (!win0)
        return;

    move_win0 (win_x, win_y);
}

void move_win0 (int x, int y) {
    move_win (win0, x, y);
    move_win_sym ();
}

void set_and_show_tsin_phoneme_at_index (int index, char *pho) {
    if (hime_display_on_the_spot_key ()) {
        if (is_win0_visible ())
            hide_win0 ();
        return;
    }

    if (button_pho && !gtk_widget_get_visible (button_pho))
        gtk_widget_show (button_pho);

    text_pho_N = pin_juyin ? 6 : 3;
    set_phoneme_at_index_in_label (label_pho, index, pho);
}

void win_tsin_disp_half_full () {
    if (label_pho == NULL)
        show_win0 ();

    if (current_fullwidth_mode ()) {
        if (hime_use_custom_theme)
            gtk_label_set_markup (GTK_LABEL (label_pho), get_full_str ());
        else
            gtk_label_set_text (GTK_LABEL (label_pho), get_full_str ());
    } else {
        clear_phonemes ();
    }
    move_win0_auto ();
}

void reset_content () {
    memset (chars, 0, sizeof (chars));
}

void clear_phonemes () {
    int i;
    for (i = 0; i < text_pho_N; i++)
        set_and_show_tsin_phoneme_at_index (i, " ");
}

/* there is a bug in gtk, if the widget is created and hasn't been processed by
   gtk_main(), the coodinate of the widget is sometimes invalid.
   We use pre-create to overcome this bug.
*/

static void mouse_char_callback (GtkWidget *widget, GdkEventButton *event, gpointer data) {
    int index = GPOINTER_TO_INT (data);
    update_tsin_cursor_index (index);
    clear_cursor (tss.last_cursor_idx);
    set_cursor (tss.c_idx);

    switch (event->button) {
    case 1:
    case 2:
        open_select_pho ();
        break;
    case 3: {
        create_phrase_save_menu (event);
        break;
    }
    }
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

void change_win0_style () {
    if (!win0 || current_hime_inner_frame == hime_inner_frame)
        return;

    current_hime_inner_frame = hime_inner_frame;
    recreate_win0 ();
}

static void create_cursor_attr () {
    if (attr_list)
        pango_attr_list_unref (attr_list);

    GdkRGBA color_bg, color_fg;
    if (hime_use_custom_theme)
        gdk_rgba_parse (&color_bg, hime_cursor_color);
    else
        gdk_rgba_parse (&color_bg, HIME_CURSOR_COLOR_DEFAULT);
    gdk_rgba_parse (&color_fg, "white");

#if GTK_CHECK_VERSION(3, 0, 0)
    // In GTK3, the color in GdkRGBA is recorded as gdouble (0.0 ~ 1.0).
    // In GTK2, the color in GdkColor is recorded as guint16 (0 ~ 65535).
    // pango_attr_background/foreground_new take guint16 as its input arguments.
    guint16 red_bg = (guint16) (color_bg.red * G_MAXUINT16);
    guint16 green_bg = (guint16) (color_bg.green * G_MAXUINT16);
    guint16 blue_bg = (guint16) (color_bg.blue * G_MAXUINT16);
    guint16 red_fg = (guint16) (color_fg.red * G_MAXUINT16);
    guint16 green_fg = (guint16) (color_fg.green * G_MAXUINT16);
    guint16 blue_fg = (guint16) (color_fg.blue * G_MAXUINT16);
#else
    guint16 red_bg = color_bg.red;
    guint16 green_bg = color_bg.green;
    guint16 blue_bg = color_bg.blue;
    guint16 red_fg = color_fg.red;
    guint16 green_fg = color_fg.green;
    guint16 blue_fg = color_fg.blue;
#endif

    attr_list = pango_attr_list_new ();
    attr_list_blank = pango_attr_list_new ();

    PangoAttribute *bg = pango_attr_background_new (red_bg, green_bg, blue_bg);
    bg->start_index = 0;
    bg->end_index = 128;
    pango_attr_list_insert (attr_list, bg);

    PangoAttribute *fg = pango_attr_foreground_new (red_fg, green_fg, blue_fg);
    fg->start_index = 0;
    fg->end_index = 128;
    pango_attr_list_insert (attr_list, fg);
}

static void set_win0_bg () {
    apply_widget_bg_color (win0);
}

void change_tsin_font_size () {
    if (!win0)
        return;

    set_label_font_size (label_pho, hime_font_size_tsin_pho_in);

    int i;
    for (i = 0; i < MAX_PH_BF_EXT; i++) {
        GtkWidget *label = chars[i].label;
        if (!label)
            continue;

        set_label_font_size (label, hime_font_size);

        apply_widget_fg_color (label);
    }

    move_win0_auto ();

    set_win0_bg ();
}
