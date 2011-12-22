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
#include "pho.h"
#include "tsin.h"
#include "gst.h"
#include "im-client/hime-im-client-attr.h"
#include "win1.h"
#include "hime-module.h"

void show_win_sym(),hide_win_sym(),move_win_sym(),toggle_win_sym();
void init_tsin_selection_win(),disp_selections(int x, int y),hide_selections_win();
void disp_arrow_up(),disp_arrow_down(), set_tsin_pho_mode();
void set_sele_text(int tN, int i, char *text, int len);
void tsin_set_eng_ch(int nmod);
int get_widget_xy(GtkWidget *win, GtkWidget *widget, int *rx, int *ry);
void get_win_size(GtkWidget *win, int *width, int *height);
void exec_hime_setup();
void load_tab_pho_file();
void clear_sele();
void send_utf8_ch(char *s);
extern gboolean force_show;
#if WIN32
extern int test_mode;
void win32_init_win(GtkWidget *);
#endif

void init_HIME_module_main_functions(HIME_module_main_functions *func)
{
  func->mf_show_win_sym = show_win_sym;
  func->mf_hide_win_sym = hide_win_sym;
  func->mf_move_win_sym = move_win_sym;
  func->mf_toggle_win_sym = toggle_win_sym;

  func->mf_init_tsin_selection_win = init_tsin_selection_win;
  func->mf_clear_sele = clear_sele;
  func->mf_disp_selections = disp_selections;
  func->mf_hide_selections_win = hide_selections_win;
  func->mf_disp_arrow_up = disp_arrow_up;
  func->mf_disp_arrow_down = disp_arrow_down;
  func->mf_set_sele_text = set_sele_text;
  func->mf_set_win1_cb = set_win1_cb;

  func->mf_tsin_set_eng_ch = tsin_set_eng_ch;
  func->mf_tsin_pho_mode = tsin_pho_mode;
  func->mf_set_tsin_pho_mode = set_tsin_pho_mode;

  func->mf_get_widget_xy = get_widget_xy;
  func->mf_get_win_size = get_win_size;
  func->mf_change_win_bg = change_win_bg;
  func->mf_set_label_font_size = set_label_font_size;
  func->mf_set_no_focus = set_no_focus;


  func->mf_current_time = current_time;

  func->mf_exec_hime_setup = exec_hime_setup;
  func->mf_hime_edit_display_ap_only = hime_edit_display_ap_only;
  func->mf_inmd_switch_popup_handler = inmd_switch_popup_handler;
  func->mf_load_tab_pho_file = load_tab_pho_file;
  func->mf_send_text = send_text;
  func->mf_send_utf8_ch = send_utf8_ch;
  func->mf_utf8_str_N = utf8_str_N;

  func->mf_phkbm = &phkbm;
  func->mf_tss = &tss;
  func->mf_tsin_chinese_english_toggle_key = &tsin_chinese_english_toggle_key;

  func->mf_hime_pop_up_win = &hime_pop_up_win;
  func->mf_hime_font_size = &hime_font_size;
  func->mf_hime_win_color_fg = &hime_win_color_fg;
  func->mf_gtab_press_full_auto_send = &gtab_press_full_auto_send;

  func->mf_hime_win_color_use = &hime_win_color_use;
  func->mf_tsin_cursor_color = &tsin_cursor_color;
  func->mf_pho_selkey = &pho_selkey;
  func->mf_force_show = &force_show;
  func->mf_win_x = &win_x;
  func->mf_win_y = &win_y;
  func->mf_win_xl = &win_xl;
  func->mf_win_yl = &win_yl;
  func->mf_dpy_xl = &dpy_xl;
  func->mf_dpy_yl = &dpy_yl;
  func->mf_pho_chars = pho_chars;

#if WIN32
  func->mf_test_mode = &test_mode;
  func->mf_win32_init_win = win32_init_win;
  func->mf__utf16_8 = __utf16_8;
#endif
}
