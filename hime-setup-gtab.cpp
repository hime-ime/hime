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


static GtkWidget *check_button_gtab_dup_select_bell,
                 *check_button_gtab_press_full_auto_send,
                 *check_button_gtab_pre_select,
                 *check_button_gtab_disp_partial_match,
                 *check_button_gtab_disp_key_codes,
                 *check_button_gtab_disp_im_name,
                 *check_button_gtab_invalid_key_in,
                 *check_button_gtab_shift_phrase_key,
                 *check_button_gtab_hide_row2,
                 *check_button_gtab_in_row1,
                 *check_button_gtab_vertical_select,
                 *check_button_gtab_unique_auto_send,
                 *check_button_gtab_que_wild_card,
                 *check_button_gtab_phrase_pre_select;

extern GtkWidget *check_button_hime_capslock_lower;
extern gboolean button_order;

struct {
  unich_t *str;
  int num;
} spc_opts[] = {
  {N_(_L("由 .gtab 指定")), GTAB_space_auto_first_none},
  {N_(_L("按空白立即送出第一字(嘸蝦米、大易)")), GTAB_space_auto_first_any},
  {N_(_L("按滿按空白送出第一字")), GTAB_space_auto_first_full},
  {N_(_L("按滿按空白不送出第一字(倉頡, 行列)")), GTAB_space_auto_first_nofull},
  { NULL, 0},
};


struct {
  unich_t *str;
  int num;
} auto_select_by_phrase_opts[] = {
  {N_(_L("由.gtab指定開啟")), GTAB_AUTO_SELECT_BY_PHRASE_AUTO},
  {N_(_L("全部開啟")), GTAB_AUTO_SELECT_BY_PHRASE_YES},
  {N_(_L("全部關閉")), GTAB_AUTO_SELECT_BY_PHRASE_NO},
  { NULL, 0},
};

void save_tsin_eng_pho_key();
static GtkWidget *hime_gtab_conf_window;
static GtkWidget *opt_spc_opts, *opt_auto_select_by_phrase;
static gboolean cb_gtab_conf_ok( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  save_tsin_eng_pho_key();
  save_hime_conf_int(GTAB_DUP_SELECT_BELL,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_dup_select_bell)));

  save_hime_conf_int(GTAB_PRE_SELECT,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_pre_select)));

  save_hime_conf_int(GTAB_PRESS_FULL_AUTO_SEND,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_press_full_auto_send)));

  save_hime_conf_int(GTAB_DISP_PARTIAL_MATCH,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_partial_match)));
  save_hime_conf_int(GTAB_DISP_KEY_CODES,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_key_codes)));

  save_hime_conf_int(GTAB_DISP_IM_NAME,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_im_name)));

  save_hime_conf_int(GTAB_INVALID_KEY_IN,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_invalid_key_in)));

  save_hime_conf_int(GTAB_SHIFT_PHRASE_KEY,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_shift_phrase_key)));

  save_hime_conf_int(GTAB_HIDE_ROW2,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_hide_row2)));

  save_hime_conf_int(GTAB_IN_ROW1,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_in_row1)));

  save_hime_conf_int(GTAB_VERTICAL_SELECT,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_vertical_select)));


  save_hime_conf_int(GTAB_UNIQUE_AUTO_SEND,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_unique_auto_send)));

  save_hime_conf_int(GTAB_QUE_WILD_CARD,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_que_wild_card)));

  save_hime_conf_int(HIME_CAPSLOCK_LOWER,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_capslock_lower)));

  save_hime_conf_int(GTAB_PHRASE_PRE_SELECT,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_phrase_pre_select)));

  int idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_spc_opts));
  save_hime_conf_int(GTAB_SPACE_AUTO_FIRST, spc_opts[idx].num);


  idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_auto_select_by_phrase));
  save_hime_conf_int(GTAB_AUTO_SELECT_BY_PHRASE, auto_select_by_phrase_opts[idx].num);

  send_hime_message(
#if UNIX
	  GDK_DISPLAY(),
#endif
	  CHANGE_FONT_SIZE);
  gtk_widget_destroy(hime_gtab_conf_window); hime_gtab_conf_window = NULL;

  return TRUE;
}


static gboolean close_gtab_conf_window( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  gtk_widget_destroy(hime_gtab_conf_window); hime_gtab_conf_window = NULL;
  return TRUE;
}

extern char utf8_edit[];
static gboolean cb_gtab_edit_append( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  load_gtab_list(FALSE);
  char *fname = inmd[default_input_method].filename;
  if (!fname)
    return TRUE;

  char append_fname[128];
#if UNIX
  sprintf(append_fname, "~/.config/hime/%s.append", fname);

  char prepare[128];
  sprintf(prepare, HIME_SCRIPT_DIR"/gtab.append_prepare %s", append_fname);
  system(prepare);

  char exec[128];

  sprintf(exec, "%s %s", utf8_edit, append_fname);
  dbg("exec %s\n", exec);
  system(exec);
#else
  strcat(strcpy(append_fname, fname), ".append");
  char user_fname[512];
  get_hime_user_fname(append_fname, user_fname);
  if (GetFileAttributesA(user_fname) == INVALID_FILE_ATTRIBUTES)
    win32exec_script("gtab.append_prepare.bat", user_fname);
  else {
    win32exec_script("utf8-edit.bat", user_fname);
  }
#endif
  return TRUE;
}

static GtkWidget *create_spc_opts()
{
  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);
  GtkWidget *label = gtk_label_new(_(_L("空白鍵選項")));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_spc_opts = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
  GtkWidget *menu_spc_opts = gtk_menu_new ();
#endif
  gtk_box_pack_start (GTK_BOX (hbox), opt_spc_opts, FALSE, FALSE, 0);

  int i, current_idx=0;

  for(i=0; spc_opts[i].str; i++) {
#if !GTK_CHECK_VERSION(2,4,0)
    GtkWidget *item = gtk_menu_item_new_with_label (_(spc_opts[i].str));
#endif

    if (spc_opts[i].num == gtab_space_auto_first)
      current_idx = i;

#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_spc_opts), _(spc_opts[i].str));
#else
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_spc_opts), item);
#endif
  }

#if !GTK_CHECK_VERSION(2,4,0)
  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_spc_opts), menu_spc_opts);
#endif
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_spc_opts), current_idx);

  return hbox;
}

static GtkWidget *create_auto_select_by_phrase_opts()
{
  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  opt_auto_select_by_phrase = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
  GtkWidget *menu_auto_select_by_phrase = gtk_menu_new ();
#endif
  gtk_box_pack_start (GTK_BOX (hbox), opt_auto_select_by_phrase, FALSE, FALSE, 0);

  int i, current_idx=0;

  for(i=0; auto_select_by_phrase_opts[i].str; i++) {
#if !GTK_CHECK_VERSION(2,4,0)
    GtkWidget *item = gtk_menu_item_new_with_label (_(auto_select_by_phrase_opts[i].str));
#endif

    if (auto_select_by_phrase_opts[i].num == gtab_auto_select_by_phrase)
      current_idx = i;

#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_auto_select_by_phrase), _(auto_select_by_phrase_opts[i].str));
#else
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_auto_select_by_phrase), item);
#endif
  }

#if !GTK_CHECK_VERSION(2,4,0)
  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_auto_select_by_phrase), menu_auto_select_by_phrase);
#endif
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_auto_select_by_phrase), current_idx);

  return hbox;
}


GtkWidget *create_en_pho_key_sel(char *s);

void create_gtab_conf_window()
{
  if (hime_gtab_conf_window) {
    gtk_window_present(GTK_WINDOW(hime_gtab_conf_window));
    return;
  }

  load_setttings();

  hime_gtab_conf_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(hime_gtab_conf_window), GTK_WIN_POS_MOUSE);
  gtk_window_set_has_resize_grip(GTK_WINDOW(hime_gtab_conf_window), FALSE);

  g_signal_connect (G_OBJECT (hime_gtab_conf_window), "delete_event",
                    G_CALLBACK (close_gtab_conf_window),
                    NULL);

  gtk_window_set_title (GTK_WINDOW (hime_gtab_conf_window), _(_L("倉頡/行列/大易設定")));
  gtk_container_set_border_width (GTK_CONTAINER (hime_gtab_conf_window), 3);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 10);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_top), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (hime_gtab_conf_window), vbox_top);

  GtkWidget *hbox_lr = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_lr, FALSE, FALSE, 0);


  GtkWidget *frame_gtab_l = gtk_frame_new(_(_L("外觀")));
  gtk_container_set_border_width (GTK_CONTAINER (frame_gtab_l), 5);
  gtk_box_pack_start (GTK_BOX (hbox_lr), frame_gtab_l, FALSE, FALSE, 0);
  GtkWidget *vbox_gtab_l = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_gtab_l), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (frame_gtab_l), vbox_gtab_l);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_gtab_l), 10);


  GtkWidget *frame_gtab_r = gtk_frame_new(_(_L("行為")));
  gtk_container_set_border_width (GTK_CONTAINER (frame_gtab_r), 5);
  gtk_box_pack_start (GTK_BOX (hbox_lr), frame_gtab_r, FALSE, FALSE, 0);
  GtkWidget *vbox_gtab_r = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_gtab_r), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (frame_gtab_r), vbox_gtab_r);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_gtab_r), 10);

#define SPC 1

  GtkWidget *hbox_gtab_pre_select = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_pre_select, FALSE, FALSE, 0);
  check_button_gtab_pre_select = gtk_check_button_new_with_label (_(_L("預覽/預選 字")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_pre_select),check_button_gtab_pre_select,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_pre_select),
     gtab_pre_select);


  GtkWidget *hbox_gtab_disp_partial_match = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_disp_partial_match, FALSE, FALSE, 0);
  check_button_gtab_disp_partial_match = gtk_check_button_new_with_label (_(_L("預選列中顯示部份符合的字")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_partial_match), check_button_gtab_disp_partial_match,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_partial_match),
     gtab_disp_partial_match);


  GtkWidget *hbox_gtab_disp_key_codes = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_disp_key_codes, FALSE, FALSE, 0);
  check_button_gtab_disp_key_codes = gtk_check_button_new_with_label (_(_L("顯示字根")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_key_codes), check_button_gtab_disp_key_codes,  FALSE, FALSE, 0);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_key_codes),
     gtab_disp_key_codes);


  GtkWidget *hbox_gtab_disp_im_name = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_disp_im_name, FALSE, FALSE, 0);
  check_button_gtab_disp_im_name = gtk_check_button_new_with_label (_(_L("顯示輸入法名稱")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_im_name), check_button_gtab_disp_im_name,  FALSE, FALSE, 0);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_im_name),
     gtab_disp_im_name);

  GtkWidget *hbox_gtab_hide_row2 = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_hide_row2, FALSE, FALSE, 0);
  check_button_gtab_hide_row2 = gtk_check_button_new_with_label (_(_L("隱藏第二列 (輸入鍵…)")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_hide_row2), check_button_gtab_hide_row2,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_hide_row2),
     gtab_hide_row2);


  GtkWidget *hbox_gtab_in_row1 = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_in_row1, FALSE, FALSE, 0);
  check_button_gtab_in_row1 = gtk_check_button_new_with_label (_(_L("將字根移至第一列")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_in_row1), check_button_gtab_in_row1,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_in_row1),
     gtab_in_row1);

  GtkWidget *hbox_gtab_vertical_select = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_l), hbox_gtab_vertical_select, FALSE, FALSE, 0);
  check_button_gtab_vertical_select = gtk_check_button_new_with_label (_(_L("垂直選擇")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_vertical_select), check_button_gtab_vertical_select,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_vertical_select),
     gtab_vertical_select);

  GtkWidget *hbox_gtab_press_full_auto_send = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_press_full_auto_send, FALSE, FALSE, 0);
  check_button_gtab_press_full_auto_send = gtk_check_button_new_with_label (_(_L("按滿字根自動送字")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_press_full_auto_send), check_button_gtab_press_full_auto_send,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_press_full_auto_send),
     gtab_press_full_auto_send);


  GtkWidget *hbox_gtab_auto_select_by_phrase = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_auto_select_by_phrase, FALSE, FALSE, 0);
  GtkWidget *label_gtab_auto_select = gtk_label_new(_(_L("由詞庫自動選字")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase), label_gtab_auto_select,  FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase), create_auto_select_by_phrase_opts(),  FALSE, FALSE, 0);
  check_button_gtab_phrase_pre_select = gtk_check_button_new_with_label (_(_L("使用預選詞")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase), check_button_gtab_phrase_pre_select,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_phrase_pre_select), gtab_phrase_pre_select);


  GtkWidget *hbox_gtab_dup_select_bell = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_dup_select_bell, FALSE, FALSE, 0);
  check_button_gtab_dup_select_bell = gtk_check_button_new_with_label (_(_L("有重複字時，發出嗶聲")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_dup_select_bell),check_button_gtab_dup_select_bell,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_dup_select_bell),
     gtab_dup_select_bell);

  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), create_spc_opts(), FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_invalid_key_in = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_invalid_key_in, FALSE, FALSE, 0);
  check_button_gtab_invalid_key_in = gtk_check_button_new_with_label (_(_L("可鍵入錯誤字根 (傳統)")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_invalid_key_in), check_button_gtab_invalid_key_in,  FALSE, FALSE, 0);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_invalid_key_in),
     gtab_invalid_key_in);


  GtkWidget *hbox_gtab_shift_phrase_key = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_shift_phrase_key, FALSE, FALSE, 0);
  check_button_gtab_shift_phrase_key = gtk_check_button_new_with_label (_(_L("可用 Shift 輸入片語 (預設為 Alt-Shift)")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_shift_phrase_key), check_button_gtab_shift_phrase_key,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_shift_phrase_key),
     gtab_shift_phrase_key);

  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), create_en_pho_key_sel(_(_L("切換[中/英]打"))), FALSE, FALSE, 0);

#if 0
  GtkWidget *hbox_hime_capslock_lower = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_hime_capslock_lower, FALSE, FALSE, 0);
  check_button_hime_capslock_lower = gtk_check_button_new_with_label (_(_L("\t用小寫字母")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_capslock_lower), check_button_hime_capslock_lower,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_capslock_lower),
     hime_capslock_lower);
#endif

  GtkWidget *hbox_gtab_unique_auto_send = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_unique_auto_send, FALSE, FALSE, 0);
  check_button_gtab_unique_auto_send = gtk_check_button_new_with_label (_(_L("唯一選擇時自動送出")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_unique_auto_send), check_button_gtab_unique_auto_send,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_unique_auto_send),
     gtab_unique_auto_send);


  GtkWidget *hbox_gtab_que_wild_card = gtk_hbox_new (FALSE, SPC);
  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), hbox_gtab_que_wild_card, FALSE, FALSE, 0);
  check_button_gtab_que_wild_card = gtk_check_button_new_with_label (_(_L("使用？萬用字元")));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_que_wild_card), check_button_gtab_que_wild_card,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_que_wild_card),
     gtab_que_wild_card);


  GtkWidget *button_edit_append = gtk_button_new_with_label(_(_L("編輯預設輸入法的使用者外加字詞")));
  gtk_box_pack_start (GTK_BOX (vbox_gtab_r), button_edit_append, FALSE, FALSE, 0);

  g_signal_connect_swapped (G_OBJECT (button_edit_append), "clicked",
                            G_CALLBACK (cb_gtab_edit_append), NULL);


  GtkWidget *hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
  gtk_grid_set_column_homogeneous(GTK_GRID(hbox_cancel_ok), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_cancel_ok, FALSE, FALSE, 0);

  GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  if (button_order)
    gtk_box_pack_end (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);
  else
    gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT (button_cancel), "clicked",
                            G_CALLBACK (close_gtab_conf_window),
                            G_OBJECT (hime_gtab_conf_window));

  GtkWidget *button_ok = gtk_button_new_from_stock (GTK_STOCK_OK);
#if !GTK_CHECK_VERSION(2,91,2)
  if (button_order)
    gtk_box_pack_end (GTK_BOX (hbox_cancel_ok), button_ok, TRUE, TRUE, 0);
  else
    gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_ok, TRUE, TRUE, 0);
#else
  if (button_order)
    gtk_grid_attach_next_to (GTK_BOX (hbox_cancel_ok), button_ok, button_cancel, GTK_POS_LEFT, 1, 1);
  else
    gtk_grid_attach_next_to (GTK_BOX (hbox_cancel_ok), button_ok, button_cancel, GTK_POS_RIGHT, 1, 1);
#endif

  g_signal_connect_swapped (G_OBJECT (button_ok), "clicked",
                            G_CALLBACK (cb_gtab_conf_ok),
                            G_OBJECT (hime_gtab_conf_window));

  GTK_WIDGET_SET_FLAGS (button_ok, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button_ok);

  gtk_widget_show_all (hime_gtab_conf_window);

  return;
}
