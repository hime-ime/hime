/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 2012 Favonia <favonia@gmail.com>
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

#include "gtab.h"

/* XXX UI states hold uncommited preference.
 * That's why we need these global variables. */
static GtkWidget *check_button_gtab_dup_select_bell,
    *opt_gtab_press_full_auto_send,
    *opt_gtab_pre_select,
    *opt_gtab_disp_partial_match,
    *check_button_gtab_disp_key_codes,
    *check_button_gtab_disp_im_name,
    *check_button_gtab_invalid_key_in,
    *check_button_gtab_shift_phrase_key,
    *check_button_gtab_hide_row2,
    *check_button_gtab_in_row1,
    *opt_gtab_vertical_select,
    *opt_gtab_unique_auto_send,
    *check_button_gtab_que_wild_card,
    *check_button_gtab_que_wild_card_asterisk,
    *check_button_gtab_pho_query,
    *check_button_gtab_phrase_pre_select;

extern GtkWidget *check_button_hime_capslock_lower;
extern gboolean button_order;

struct {
    unich_t *str;
    int num;
} spc_opts[] = {
    {N_ ("Assigned by .gtab"), GTAB_space_auto_first_none},
    {N_ ("Send first character within Liu's IM"), GTAB_space_auto_first_any},
    {N_ ("Send first character within character-selection area"), GTAB_space_auto_first_full},
    {N_ ("Does not send first character"), GTAB_space_auto_first_nofull},
    {NULL, 0},
};

struct {
    unich_t *str;
    int num;
} auto_select_by_phrase_opts[] = {
    {N_ ("Assigned by .gtab"), GTAB_OPTION_AUTO},
    {N_ ("Enable all"), GTAB_OPTION_YES},
    {N_ ("Disable all"), GTAB_OPTION_NO},
    {NULL, 0},
};

void save_tsin_eng_pho_key ();
static GtkWidget *gtab_widget;
static GtkWidget *opt_spc_opts, *opt_auto_select_by_phrase;

void save_menu_val (char *config, GtkWidget *opt) {
    int idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt));
    save_hime_conf_int (config, auto_select_by_phrase_opts[idx].num);
}

void save_gtab_conf () {
    if (gtab_widget == NULL) {
        fprintf (stderr, "save_gtab_conf: gtab_widget is NULL!\n");
        return;
    }

    save_tsin_eng_pho_key ();
    save_hime_conf_int (GTAB_DUP_SELECT_BELL,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_gtab_dup_select_bell)));

    save_menu_val (GTAB_PRE_SELECT, opt_gtab_pre_select);

    save_menu_val (GTAB_DISP_PARTIAL_MATCH, opt_gtab_disp_partial_match);

    save_hime_conf_int (GTAB_DISP_KEY_CODES,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_gtab_disp_key_codes)));

    save_hime_conf_int (GTAB_DISP_IM_NAME,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_gtab_disp_im_name)));

    save_hime_conf_int (GTAB_INVALID_KEY_IN,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_gtab_invalid_key_in)));

    save_hime_conf_int (GTAB_SHIFT_PHRASE_KEY,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_gtab_shift_phrase_key)));

    save_hime_conf_int (GTAB_HIDE_ROW2,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_gtab_hide_row2)));

    save_hime_conf_int (GTAB_IN_ROW1,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_gtab_in_row1)));

    save_menu_val (GTAB_VERTICAL_SELECT, opt_gtab_vertical_select);

    save_hime_conf_int (GTAB_QUE_WILD_CARD,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_gtab_que_wild_card)));

    save_hime_conf_int (GTAB_QUE_WILD_CARD_ASTERISK,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_gtab_que_wild_card_asterisk)));

    save_hime_conf_int (GTAB_PHO_QUERY,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_gtab_pho_query)));

    save_hime_conf_int (HIME_CAPSLOCK_LOWER,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_hime_capslock_lower)));

    save_hime_conf_int (GTAB_PHRASE_PRE_SELECT,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_gtab_phrase_pre_select)));

    int idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_spc_opts));
    save_hime_conf_int (GTAB_SPACE_AUTO_FIRST, spc_opts[idx].num);

    save_menu_val (GTAB_AUTO_SELECT_BY_PHRASE, opt_auto_select_by_phrase);

    save_menu_val (GTAB_PRESS_FULL_AUTO_SEND, opt_gtab_press_full_auto_send);

    save_menu_val (GTAB_UNIQUE_AUTO_SEND, opt_gtab_unique_auto_send);

    save_omni_config ();
    send_hime_message (GDK_DISPLAY (), CHANGE_FONT_SIZE);
}

void destroy_gtab_widget () {
    gtk_widget_destroy (gtab_widget);
    gtab_widget = NULL;
}

extern char utf8_edit[];
static gboolean cb_gtab_edit_append (GtkWidget *widget,
                                     GdkEvent *event,
                                     gpointer data) {
    load_gtab_list (FALSE);
    char *fname = inmd[default_input_method].filename;
    if (!fname)
        return TRUE;

    char append_fname[128];
    snprintf (append_fname, sizeof (append_fname), "~/.config/hime/%s.append", fname);

    char prepare[256];
    snprintf (prepare, sizeof (prepare), HIME_SCRIPT_DIR "/gtab.append_prepare %s", append_fname);
    system (prepare);

    char exec[256];

    snprintf (exec, sizeof (exec), "%s %s", utf8_edit, append_fname);
    dbg ("exec %s\n", exec);
    system (exec);
    return TRUE;
}

static GtkWidget *create_spc_opts () {
    GtkWidget *frame = gtk_frame_new (_ ("Behavior of Space key"));

    opt_spc_opts = gtk_combo_box_text_new ();
    gtk_container_add (GTK_CONTAINER (frame), opt_spc_opts);

    int i, current_idx = 0;

    for (i = 0; spc_opts[i].str; i++) {
        if (spc_opts[i].num == gtab_space_auto_first)
            current_idx = i;
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (opt_spc_opts), _ (spc_opts[i].str));
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (opt_spc_opts), current_idx);

    return frame;
}

static GtkWidget *create_auto_select_by_phrase_opts (GtkWidget **out, int val) {
    *out = gtk_combo_box_text_new ();

    int i, current_idx = 0;

    for (i = 0; auto_select_by_phrase_opts[i].str; i++) {
        if (auto_select_by_phrase_opts[i].num == val)
            current_idx = i;
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (*out), _ (auto_select_by_phrase_opts[i].str));
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (*out), current_idx);

    return *out;
}

GtkWidget *create_en_pho_key_sel (char *s, gint index);

GtkWidget *create_gtab_widget () {
    if (gtab_widget != NULL)
        fprintf (stderr, "create_gtab_widget: gtab_widget was not NULL!\n");

    load_settings ();

    GtkWidget *vbox_top = gtk_vbox_new (FALSE, 10);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_top), GTK_ORIENTATION_VERTICAL);
    gtab_widget = vbox_top;

    GtkWidget *hbox_lr = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_lr, FALSE, FALSE, 0);

    GtkWidget *frame_gtab_l = gtk_frame_new (_ ("Appearance Settings"));
    gtk_container_set_border_width (GTK_CONTAINER (frame_gtab_l), 10);
    gtk_box_pack_start (GTK_BOX (hbox_lr), frame_gtab_l, TRUE, TRUE, 0);
    GtkWidget *vbox_gtab_l = gtk_vbox_new (FALSE, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_gtab_l), GTK_ORIENTATION_VERTICAL);
    gtk_container_add (GTK_CONTAINER (frame_gtab_l), vbox_gtab_l);

    GtkWidget *frame_gtab_r = gtk_frame_new (_ ("Behavior Settings"));
    gtk_container_set_border_width (GTK_CONTAINER (frame_gtab_r), 10);
    gtk_box_pack_start (GTK_BOX (hbox_lr), frame_gtab_r, TRUE, TRUE, 0);
    GtkWidget *vbox_gtab_r = gtk_vbox_new (FALSE, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_gtab_r), GTK_ORIENTATION_VERTICAL);

    gtk_container_add (GTK_CONTAINER (frame_gtab_r), vbox_gtab_r);

#define SPC 1

    GtkWidget *hbox_gtab_pre_select = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_pre_select, FALSE, FALSE, 0);
    opt_gtab_pre_select = gtk_label_new (_ ("Preview choices"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_pre_select), opt_gtab_pre_select, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_gtab_pre_select), create_auto_select_by_phrase_opts (&opt_gtab_pre_select, gtab_pre_select), FALSE, FALSE, 0);

    GtkWidget *hbox_gtab_disp_partial_match = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_disp_partial_match, FALSE, FALSE, 0);
    opt_gtab_disp_partial_match = gtk_label_new (_ ("Preview keycode-partial-matched choices"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_partial_match), opt_gtab_disp_partial_match, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_partial_match), create_auto_select_by_phrase_opts (&opt_gtab_disp_partial_match, gtab_disp_partial_match), FALSE, FALSE, 0);

    GtkWidget *hbox_gtab_vertical_select = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_vertical_select, FALSE, FALSE, 0);
    GtkWidget *label_gtab_vertical_select = gtk_label_new (_ ("Vertical selection window"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_vertical_select), label_gtab_vertical_select, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_gtab_vertical_select), create_auto_select_by_phrase_opts (&opt_gtab_vertical_select, gtab_vertical_select), FALSE, FALSE, 0);

    GtkWidget *hbox_gtab_disp_key_codes = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_disp_key_codes, FALSE, FALSE, 0);
    check_button_gtab_disp_key_codes = gtk_check_button_new_with_label (_ ("顯示拆碼"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_key_codes), check_button_gtab_disp_key_codes, FALSE, FALSE, 0);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_gtab_disp_key_codes),
                                  gtab_disp_key_codes);
    GtkWidget *hbox_gtab_disp_im_name = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_disp_im_name, FALSE, FALSE, 0);
    check_button_gtab_disp_im_name = gtk_check_button_new_with_label (_ ("Show input method names"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_im_name), check_button_gtab_disp_im_name, FALSE, FALSE, 0);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_gtab_disp_im_name),
                                  gtab_disp_im_name);

    GtkWidget *hbox_gtab_hide_row2 = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_hide_row2, FALSE, FALSE, 0);
    check_button_gtab_hide_row2 = gtk_check_button_new_with_label (_ ("Hide second row of input window"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_hide_row2), check_button_gtab_hide_row2, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_gtab_hide_row2),
                                  gtab_hide_row2);

    GtkWidget *hbox_gtab_in_row1 = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_in_row1, FALSE, FALSE, 0);
    check_button_gtab_in_row1 = gtk_check_button_new_with_label (_ ("Move word components to the first row"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_in_row1), check_button_gtab_in_row1, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_gtab_in_row1),
                                  gtab_in_row1);

    GtkWidget *hbox_gtab_press_full_auto_send = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_press_full_auto_send, FALSE, FALSE, 0);
    GtkWidget *label_gtab_gtab_press_full_auto_send = gtk_label_new (_ ("Auto-send when keycodes are filled"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_press_full_auto_send), label_gtab_gtab_press_full_auto_send, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_gtab_press_full_auto_send), create_auto_select_by_phrase_opts (&opt_gtab_press_full_auto_send, gtab_press_full_auto_send), FALSE, FALSE, 0);

    GtkWidget *hbox_gtab_auto_select_by_phrase = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_auto_select_by_phrase, FALSE, FALSE, 0);
    GtkWidget *label_gtab_auto_select = gtk_label_new (_ ("Auto-select by Tsin's database"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase), label_gtab_auto_select, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase), create_auto_select_by_phrase_opts (&opt_auto_select_by_phrase, gtab_auto_select_by_phrase), FALSE, FALSE, 0);
    check_button_gtab_phrase_pre_select = gtk_check_button_new_with_label (_ ("Enable preselection"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase), check_button_gtab_phrase_pre_select, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_gtab_phrase_pre_select), gtab_phrase_pre_select);

    GtkWidget *hbox_gtab_dup_select_bell = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_dup_select_bell, FALSE, FALSE, 0);
    check_button_gtab_dup_select_bell = gtk_check_button_new_with_label (_ ("Beep on repeated word"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_dup_select_bell), check_button_gtab_dup_select_bell, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_gtab_dup_select_bell),
                                  gtab_dup_select_bell);

    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), create_spc_opts (), FALSE, FALSE, 0);

    GtkWidget *hbox_gtab_invalid_key_in = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_invalid_key_in, FALSE, FALSE, 0);
    check_button_gtab_invalid_key_in = gtk_check_button_new_with_label (_ ("Allow typing mistake"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_invalid_key_in), check_button_gtab_invalid_key_in, FALSE, FALSE, 0);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_gtab_invalid_key_in),
                                  gtab_invalid_key_in);

    GtkWidget *hbox_gtab_shift_phrase_key = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_shift_phrase_key, FALSE, FALSE, 0);
    check_button_gtab_shift_phrase_key = gtk_check_button_new_with_label (_ ("Single Shift key to input Alt-Shift-phrases"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_shift_phrase_key), check_button_gtab_shift_phrase_key, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_gtab_shift_phrase_key),
                                  gtab_shift_phrase_key);

    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), create_en_pho_key_sel (_ ("Toggle [中/英] input"), 1), FALSE, FALSE, 0);

#if 0
  GtkWidget *hbox_hime_capslock_lower = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_hime_capslock_lower, FALSE, FALSE, 0);
  check_button_hime_capslock_lower = gtk_check_button_new_with_label (_("\tUse lower case letters"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_capslock_lower), check_button_hime_capslock_lower,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_capslock_lower),
     hime_capslock_lower);
#endif

    GtkWidget *hbox_gtab_unique_auto_send = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_unique_auto_send, FALSE, FALSE, 0);
    GtkWidget *label_gtab_unique_auto_send = gtk_label_new (_ ("Auto-send when only one choice matched"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_unique_auto_send), label_gtab_unique_auto_send, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_gtab_unique_auto_send), create_auto_select_by_phrase_opts (&opt_gtab_unique_auto_send, gtab_unique_auto_send), FALSE, FALSE, 0);

    GtkWidget *hbox_gtab_que_wild_card = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_que_wild_card, FALSE, FALSE, 0);
    check_button_gtab_que_wild_card = gtk_check_button_new_with_label (_ ("Use ? as wildcard"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_que_wild_card), check_button_gtab_que_wild_card, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_gtab_que_wild_card),
                                  gtab_que_wild_card);

    GtkWidget *hbox_gtab_que_wild_card_asterisk = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_que_wild_card_asterisk, FALSE, FALSE, 0);
    check_button_gtab_que_wild_card_asterisk = gtk_check_button_new_with_label (_ ("Use * as wildcard"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_que_wild_card_asterisk), check_button_gtab_que_wild_card_asterisk, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_gtab_que_wild_card_asterisk),
                                  gtab_que_wild_card_asterisk);

    GtkWidget *hbox_gtab_pho_query = gtk_hbox_new (FALSE, SPC);
    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_pho_query, FALSE, FALSE, 0);
    check_button_gtab_pho_query = gtk_check_button_new_with_label (_ ("Use ` to query same pronunciation word"));
    gtk_box_pack_start (GTK_BOX (hbox_gtab_pho_query), check_button_gtab_pho_query, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_gtab_pho_query),
                                  gtab_pho_query);

    GtkWidget *button_edit_append = gtk_button_new_with_label (_ ("Add definitions for default input method"));
    gtk_box_pack_start (GTK_BOX (vbox_gtab_r), button_edit_append, FALSE, FALSE, 0);

    g_signal_connect_swapped (G_OBJECT (button_edit_append), "clicked",
                              G_CALLBACK (cb_gtab_edit_append), NULL);

    return gtab_widget;
}
