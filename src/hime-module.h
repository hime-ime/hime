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

// optional functions/data for hime module to use, please refer to anthy.cpp
#if UNIX && defined(__cplusplus)
extern "C" {
#endif
typedef struct {
  void (*mf_show_win_sym)();
  void (*mf_hide_win_sym)();
  void (*mf_move_win_sym)();
  void (*mf_toggle_win_sym)();

  // display in tsin's selection window
  void (*mf_init_tsin_selection_win)();
  void (*mf_clear_sele)();
  void (*mf_disp_selections)(int x, int y);
  void (*mf_hide_selections_win)();
  void (*mf_disp_arrow_up)();
  void (*mf_disp_arrow_down)();
  void (*mf_set_sele_text)(int selN, int sel_idx, char *text, int len);
  void (*mf_set_win1_cb)(cb_selec_by_idx_t selc_by_idx, cb_page_ud_t cb_page_up, cb_page_ud_t cb_page_down);

  void (*mf_tsin_set_eng_ch)(int nmod);
  void (*mf_set_tsin_pho_mode)();
  gboolean (*mf_tsin_pho_mode)();

  int (*mf_get_widget_xy)(GtkWidget *win, GtkWidget *widget, int *rx, int *ry);
  void (*mf_get_win_size)(GtkWidget *win, int *width, int *height);
  void (*mf_change_win_bg)(GtkWidget *win);
  void (*mf_set_label_font_size)(GtkWidget *label, int size);
  void (*mf_set_no_focus)(GtkWidget *win);

  gint64 (*mf_current_time)();

  void (*mf_exec_hime_setup)();
  gboolean (*mf_hime_edit_display_ap_only)();
  gint (*mf_inmd_switch_popup_handler)(GtkWidget *widget, GdkEvent *event);
  void (*mf_load_tab_pho_file)();
  int (*mf_utf8_str_N)(char *str);

  // call this function to return the string
  void (*mf_send_text)(char *str);
  void (*mf_send_utf8_ch)(char *str);

  PHOKBM *mf_phkbm;
  TSIN_ST *mf_tss;
  int *mf_tsin_chinese_english_toggle_key;
  char **mf_pho_chars;

  int *mf_hime_pop_up_win;
  int *mf_hime_font_size, *mf_hime_win_color_use;
  int *mf_gtab_press_full_auto_send;
  char **mf_hime_win_color_fg, **mf_pho_selkey, **mf_tsin_cursor_color;
  gboolean *mf_force_show;
  int *mf_win_x, *mf_win_y, *mf_win_xl, *mf_win_yl, *mf_dpy_xl, *mf_dpy_yl;
#if WIN32
  int *mf_test_mode;
  void (*mf_win32_init_win)(GtkWidget *);
  char *(*mf__utf16_8)(wchar_t *s);
#endif
} HIME_module_main_functions;
#if UNIX && defined(__cplusplus)
}
#endif

void init_HIME_module_main_functions(HIME_module_main_functions *func);
