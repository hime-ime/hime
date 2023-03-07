/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
 * Copyright (C) 2012 Favonia <favonia@gmail.com>
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

#include "pho-kbm-name.h"
#include "pho.h"

/* "5849302-67" is Dvorak Simplified Keyboard remapped under QWERTY keyboard
 * Please see: http://en.wikipedia.org/wiki/Dvorak_Simplified_Keyboard
 *             https://github.com/hime-ime/hime/issues/62
 */
struct {
    char *kstr;
    int RL;
} selkeys[] = {
    {"123456789", 0}, {"asdfghjkl;", 0}, {"asdfzxcv", 0}, {"fdsavcxz", 1}, {"rewfdsvcx", 1}, {"fdsvcxrew", 1}, {"3825416790", 1}, {"5849302-67", 1}, {NULL}};

/* XXX UI states hold uncommited preference.
 * That's why we need these global variables. */
static GtkWidget *check_button_tsin_phrase_pre_select,
    *check_button_phonetic_char_dynamic_sequence,
    *check_button_pho_hide_row2,
    *check_button_pho_in_row1,
    *check_button_phonetic_huge_tab,
    *check_button_tsin_tone_char_input,
    *check_button_tsin_tab_phrase_end,
    *check_button_tsin_tail_select_key,
    *check_button_tsin_buffer_editing_mode,
    *check_button_tsin_use_pho_near,
    *spinner_tsin_buffer_size,
    *spinner_pho_candidate_col_N;

GtkWidget *check_button_hime_capslock_lower;

static GtkWidget *opt_kbm_opts, *opt_selkeys;
extern gboolean button_order;

static struct {
    unich_t *name;
    int key;
} tsin_space_options[] = {
    {N_ ("Select other same phonetic characters"), TSIN_SPACE_OPT_SELECT_CHAR},
    {N_ ("Send space to Tsin buffer"), TSIN_SPACE_OPT_INPUT}};
int tsin_space_optionsN = sizeof (tsin_space_options) / sizeof (tsin_space_options[0]);

extern char *pho_speaker[16];
extern int pho_speakerN;

int get_current_speaker_idx () {
    int i;

    for (i = 0; i < pho_speakerN; i++)
        if (!strcmp (pho_speaker[i], phonetic_speak_sel))
            return i;

    return 0;
}

void save_tsin_eng_pho_key () {
    save_hime_conf_int (HIME_CAPSLOCK_LOWER,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_hime_capslock_lower)));
}

static GtkWidget *kbm_widget = NULL;

static int new_select_idx_tsin_space_opt;
// static GdkColor tsin_phrase_line_gcolor;

void save_kbm_conf () {
    if (kbm_widget == NULL) {
        fprintf (stderr, "save_kbm_conf: kbm_widget is NULL!\n");
        return;
    }

    int idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_kbm_opts));

    int idx_selkeys = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_selkeys));

    pho_candidate_col_N = (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (spinner_pho_candidate_col_N));

    if (pho_candidate_col_N > strlen (selkeys[idx_selkeys].kstr))
        pho_candidate_col_N = strlen (selkeys[idx_selkeys].kstr);

    dbg ("pho_candidate_col_N %d\n", pho_candidate_col_N);

    char tt[128];
    snprintf (tt, sizeof (tt), "%s %s %d %d", kbm_sel[idx].kbm, selkeys[idx_selkeys].kstr, pho_candidate_col_N, selkeys[idx_selkeys].RL);

    char phokbm_name[128];
    get_hime_conf_fstr (PHONETIC_KEYBOARD, phokbm_name, "");

    if (strcmp (phokbm_name, tt)) {
        save_hime_conf_str (PHONETIC_KEYBOARD_BAK, phokbm_name);
    }
    save_hime_conf_str (PHONETIC_KEYBOARD, tt);

    save_tsin_eng_pho_key ();

    save_hime_conf_int (TSIN_SPACE_OPT,
                        tsin_space_options[new_select_idx_tsin_space_opt].key);

    save_hime_conf_int (TSIN_PHRASE_PRE_SELECT,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_tsin_phrase_pre_select)));

    save_hime_conf_int (PHONETIC_CHAR_DYNAMIC_SEQUENCE,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_phonetic_char_dynamic_sequence)));
    save_hime_conf_int (PHO_HIDE_ROW2,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_pho_hide_row2)));

    save_hime_conf_int (PHO_IN_ROW1,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_pho_in_row1)));

    save_hime_conf_int (PHONETIC_HUGE_TAB,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_phonetic_huge_tab)));

    save_hime_conf_int (TSIN_TONE_CHAR_INPUT,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_tsin_tone_char_input)));

    save_hime_conf_int (TSIN_USE_PHO_NEAR,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_tsin_use_pho_near)));

    save_hime_conf_int (TSIN_TAB_PHRASE_END,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_tsin_tab_phrase_end)));

    save_hime_conf_int (TSIN_TAIL_SELECT_KEY,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_tsin_tail_select_key)));

    save_hime_conf_int (TSIN_BUFFER_EDITING_MODE,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_tsin_buffer_editing_mode)));

    tsin_buffer_size = (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (spinner_tsin_buffer_size));
    save_hime_conf_int (TSIN_BUFFER_SIZE, tsin_buffer_size);

#if 0
  gchar *cstr;
  cstr = gtk_color_selection_palette_to_string(&tsin_phrase_line_gcolor, 1);
  dbg("color %s\n", cstr);
  save_hime_conf_str(TSIN_PHRASE_LINE_COLOR, cstr);
  g_free(cstr);
#endif

    save_omni_config ();
    /* caleb- does found where "reload kbm" is used.
     * caleb- think the send_hime_message() here does nothing.
     */
    send_hime_message (GDK_DISPLAY (), "reload kbm");
}

void destroy_kbm_widget () {
    gtk_widget_destroy (kbm_widget);
    kbm_widget = NULL;
}

#if 0
static void callback_button_clicked_tsin_sw( GtkWidget *widget, gpointer data)
{
  new_select_idx_tsin_sw = (int) data;
}
#endif

static void callback_button_clicked_tsin_space_opt (GtkWidget *widget, gpointer data) {
    new_select_idx_tsin_space_opt = (gsize) data;
}

static int get_current_kbm_idx () {
    int i;
    for (i = 0; kbm_sel[i].kbm; i++)
        if (!strcmp (kbm_sel[i].kbm, pho_kbm_name)) {
            return i;
        }

    p_err ("phonetic-keyboard->%s is not valid", pho_kbm_name);
    return 0;
}

static int get_currnet_tsin_space_option_idx () {
    int i;
    for (i = 0; i < tsin_space_optionsN; i++)
        if (tsin_space_options[i].key == tsin_space_opt)
            return i;

    p_err ("tsin-space-opt->%d is not valid", tsin_space_opt);
    return -1;
}

// static GtkWidget *da_phrase_line;

static GtkWidget *create_kbm_opts () {

    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);

    opt_kbm_opts = gtk_combo_box_text_new ();
    gtk_box_pack_start (GTK_BOX (hbox), opt_kbm_opts, FALSE, FALSE, 0);

    int i;
    int current_idx = get_current_kbm_idx ();

    for (i = 0; kbm_sel[i].name; i++) {
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (opt_kbm_opts), _ (kbm_sel[i].name));
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (opt_kbm_opts), current_idx);

    opt_selkeys = gtk_combo_box_text_new ();
    gtk_box_pack_start (GTK_BOX (hbox), opt_selkeys, FALSE, FALSE, 0);

    current_idx = 0;
    for (i = 0; selkeys[i].kstr; i++) {
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (opt_selkeys), selkeys[i].kstr);
        if (!strcmp (selkeys[i].kstr, pho_selkey))
            current_idx = i;
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (opt_selkeys), current_idx);

    GtkAdjustment *adj =
        (GtkAdjustment *) gtk_adjustment_new (pho_candidate_col_N, 1, 10, 1.0, 1.0, 0.0);
    spinner_pho_candidate_col_N = gtk_spin_button_new (adj, 0, 0);
    gtk_box_pack_start (GTK_BOX (hbox), spinner_pho_candidate_col_N, FALSE, FALSE, 0);

    return hbox;
}

GtkWidget *create_en_pho_key_sel (char *s, gint index) {
    GtkWidget *frame_tsin_sw = gtk_frame_new (s);
    GtkWidget *vbox_tsin_sw = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_tsin_sw), GTK_ORIENTATION_VERTICAL);
    gtk_container_add (GTK_CONTAINER (frame_tsin_sw), vbox_tsin_sw);
    gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_sw), 1);
    GtkWidget *hbox_hime_capslock_lower = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_tsin_sw), hbox_hime_capslock_lower, FALSE, FALSE, 0);
    check_button_hime_capslock_lower = gtk_check_button_new_with_label (_ ("Lower case when CapsLock is on"));
    gtk_box_pack_start (GTK_BOX (hbox_hime_capslock_lower), check_button_hime_capslock_lower, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_hime_capslock_lower), hime_capslock_lower);

    return frame_tsin_sw;
}

void load_settings ();

GtkWidget *create_kbm_widget () {
    if (kbm_widget != NULL)
        fprintf (stderr, "create_kbm_widget: kbm_widget was not NULL!\n");

    load_settings ();

    GtkWidget *vbox_top = gtk_box_new (GTK_ORIENTATION_VERTICAL, 3);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_top), GTK_ORIENTATION_VERTICAL);
    kbm_widget = vbox_top;

    GtkWidget *hbox_lr = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_lr, FALSE, FALSE, 0);

    GtkWidget *vbox_l = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_l), GTK_ORIENTATION_VERTICAL);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_l), 10);
    gtk_box_pack_start (GTK_BOX (hbox_lr), vbox_l, TRUE, TRUE, 10);

    GtkWidget *vbox_r = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_r), GTK_ORIENTATION_VERTICAL);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_r), 10);
    gtk_box_pack_start (GTK_BOX (hbox_lr), vbox_r, TRUE, TRUE, 10);

    GtkWidget *frame_kbm = gtk_frame_new (_ ("Keymap/Selection keys/Candidates per line"));
    gtk_box_pack_start (GTK_BOX (vbox_l), frame_kbm, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_kbm), 1);
    gtk_container_add (GTK_CONTAINER (frame_kbm), create_kbm_opts ());

    gtk_box_pack_start (GTK_BOX (vbox_l), create_en_pho_key_sel (_ ("(Tsin) Toggle [中/英] input"), 0), FALSE, FALSE, 0);

    GtkWidget *frame_tsin_space_opt = gtk_frame_new (_ ("(Tsin) When pressing spacebar..."));
    gtk_box_pack_start (GTK_BOX (vbox_l), frame_tsin_space_opt, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_space_opt), 1);

    GtkWidget *box_tsin_space_opt = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (box_tsin_space_opt), GTK_ORIENTATION_VERTICAL);
    gtk_container_add (GTK_CONTAINER (frame_tsin_space_opt), box_tsin_space_opt);
    gtk_container_set_border_width (GTK_CONTAINER (box_tsin_space_opt), 1);

    GSList *group_tsin_space_opt = NULL;
    int current_idx = get_currnet_tsin_space_option_idx ();
    new_select_idx_tsin_space_opt = current_idx;

    gsize i;
    for (i = 0; i < tsin_space_optionsN; i++) {
        GtkWidget *button = gtk_radio_button_new_with_label (group_tsin_space_opt, _ (tsin_space_options[i].name));
        gtk_box_pack_start (GTK_BOX (box_tsin_space_opt), button, FALSE, FALSE, 0);

        group_tsin_space_opt = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

        g_signal_connect (G_OBJECT (button), "clicked",
                          G_CALLBACK (callback_button_clicked_tsin_space_opt), (gpointer) i);

        if (i == current_idx)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    }

    GtkWidget *hbox_tsin_phrase_pre_select = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_l), hbox_tsin_phrase_pre_select, FALSE, FALSE, 1);
    check_button_tsin_phrase_pre_select = gtk_check_button_new_with_label (_ ("Enable preselection window in Tsin"));
    gtk_box_pack_start (GTK_BOX (hbox_tsin_phrase_pre_select), check_button_tsin_phrase_pre_select, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check_button_tsin_phrase_pre_select), tsin_phrase_pre_select);

    GtkWidget *hbox_phonetic_char_dynamic_sequence = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_l), hbox_phonetic_char_dynamic_sequence, FALSE, FALSE, 1);
    check_button_phonetic_char_dynamic_sequence = gtk_check_button_new_with_label (_ ("Resorting choices by user's usage frequency"));
    gtk_box_pack_start (GTK_BOX (hbox_phonetic_char_dynamic_sequence), check_button_phonetic_char_dynamic_sequence, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check_button_phonetic_char_dynamic_sequence), phonetic_char_dynamic_sequence);

    GtkWidget *hbox_pho_hide_row2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_r), hbox_pho_hide_row2, FALSE, FALSE, 1);
    check_button_pho_hide_row2 = gtk_check_button_new_with_label (_ ("Hide second row of Bopomofo's window"));
    gtk_box_pack_start (GTK_BOX (hbox_pho_hide_row2), check_button_pho_hide_row2, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check_button_pho_hide_row2), pho_hide_row2);

    GtkWidget *hbox_pho_in_row1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_r), hbox_pho_in_row1, FALSE, FALSE, 1);
    check_button_pho_in_row1 = gtk_check_button_new_with_label (_ ("Move keycodes to first row of Bopomofo's window"));
    gtk_box_pack_start (GTK_BOX (hbox_pho_in_row1), check_button_pho_in_row1, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check_button_pho_in_row1), pho_in_row1);

    GtkWidget *hbox_phonetic_huge_tab = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_r), hbox_phonetic_huge_tab, FALSE, FALSE, 1);
    check_button_phonetic_huge_tab = gtk_check_button_new_with_label (_ ("Enable more UTF-8 characters"));
    gtk_box_pack_start (GTK_BOX (hbox_phonetic_huge_tab), check_button_phonetic_huge_tab, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check_button_phonetic_huge_tab), phonetic_huge_tab);

    GtkWidget *hbox_tsin_tone_char_input = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_r), hbox_tsin_tone_char_input, FALSE, FALSE, 1);
    check_button_tsin_tone_char_input = gtk_check_button_new_with_label (_ ("Enable tone symbols in Tsin"));
    gtk_box_pack_start (GTK_BOX (hbox_tsin_tone_char_input), check_button_tsin_tone_char_input, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check_button_tsin_tone_char_input), tsin_tone_char_input);

    GtkWidget *hbox_tsin_tab_phrase_end = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_box_pack_start (GTK_BOX (vbox_r), hbox_tsin_tab_phrase_end, FALSE, FALSE, 1);
    check_button_tsin_tab_phrase_end = gtk_check_button_new_with_label (_ ("Break sentence with Escape/Tab key"));
    gtk_box_pack_start (GTK_BOX (hbox_tsin_tab_phrase_end), check_button_tsin_tab_phrase_end, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check_button_tsin_tab_phrase_end), tsin_tab_phrase_end);

    GtkWidget *hbox_tsin_tail_select_key = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_r), hbox_tsin_tail_select_key, FALSE, FALSE, 1);
    check_button_tsin_tail_select_key = gtk_check_button_new_with_label (_ ("Show selection key behind choices"));
    gtk_box_pack_start (GTK_BOX (hbox_tsin_tail_select_key), check_button_tsin_tail_select_key, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check_button_tsin_tail_select_key), tsin_tail_select_key);

    GtkWidget *hbox_tsin_buffer_editing_mode = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_r), hbox_tsin_buffer_editing_mode, FALSE, FALSE, 1);
    check_button_tsin_buffer_editing_mode = gtk_check_button_new_with_label (_ ("\\ key to toggle jkx keys editing mode"));
    gtk_box_pack_start (GTK_BOX (hbox_tsin_buffer_editing_mode), check_button_tsin_buffer_editing_mode, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check_button_tsin_buffer_editing_mode), tsin_buffer_editing_mode);

    GtkWidget *hbox_tsin_use_pho_near = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_r), hbox_tsin_use_pho_near, FALSE, FALSE, 1);
    check_button_tsin_use_pho_near = gtk_check_button_new_with_label (_ ("Press up-arrow to query similar pronunciation words"));
    gtk_box_pack_start (GTK_BOX (hbox_tsin_use_pho_near), check_button_tsin_use_pho_near, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check_button_tsin_use_pho_near), tsin_use_pho_near);

    GtkWidget *frame_tsin_buffer_size = gtk_frame_new (_ ("Tsin's buffer size"));
    gtk_box_pack_start (GTK_BOX (vbox_r), frame_tsin_buffer_size, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_buffer_size), 1);
    GtkAdjustment *adj_gtab_in =
        (GtkAdjustment *) gtk_adjustment_new (tsin_buffer_size, 10.0, MAX_PH_BF, 1.0, 1.0, 0.0);
    spinner_tsin_buffer_size = gtk_spin_button_new (adj_gtab_in, 0, 0);
    gtk_container_add (GTK_CONTAINER (frame_tsin_buffer_size), spinner_tsin_buffer_size);

    return kbm_widget;
}
