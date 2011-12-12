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

extern gboolean is_chs;

int hime_font_size, hime_font_size_tsin_presel, hime_font_size_symbol;
int hime_font_size_pho_near, hime_font_size_gtab_in, hime_font_size_win_kbm, hime_font_size_win_kbm_en;
int hime_win_color_use, hime_single_state;
int hime_remote_client;
char *default_input_method_str;
int default_input_method;
int left_right_button_tips;
int hime_im_toggle_keys, hime_bell_off;
int hime_capslock_lower, hime_eng_phrase_enabled, hime_init_im_enabled;
int hime_win_sym_click_close, hime_edit_display, hime_win32_icon;
int hime_on_the_spot_key, hime_tray_hf_win_kbm;

int gtab_dup_select_bell;
int gtab_space_auto_first;
int gtab_auto_select_by_phrase;
int gtab_press_full_auto_send;
int gtab_pre_select, gtab_phrase_pre_select;
int gtab_disp_partial_match;
int gtab_disp_key_codes;
int gtab_disp_im_name;
int gtab_invalid_key_in;
int gtab_shift_phrase_key;
int gtab_hide_row2;
int gtab_in_row1;
int gtab_vertical_select;
int gtab_unique_auto_send;
int gtab_que_wild_card, gtab_in_area_button;

int tsin_phrase_pre_select, tsin_tone_char_input;
int tsin_capslock_upper, tsin_use_pho_near;
int phonetic_char_dynamic_sequence;
int phonetic_huge_tab;
int phonetic_speak;
char *phonetic_speak_sel, *hime_str_im_cycle;
int tsin_chinese_english_toggle_key;
int hime_font_size_tsin_pho_in;
int tsin_space_opt;
int tsin_buffer_size, tsin_tail_select_key;
int tsin_buffer_editing_mode;
int hime_shift_space_eng_full;
char *tsin_phrase_line_color;
char *tsin_cursor_color, *hime_sel_key_color;
unich_t eng_full_str[]=_L("[英/全]");
unich_t cht_full_str[]=_L("[全]");
char *eng_color_full_str, *cht_color_full_str;
int tsin_tab_phrase_end;
int hime_input_style, hime_root_x, hime_root_y, hime_pop_up_win;
int hime_inner_frame;
char *hime_font_name, *hime_win_color_fg, *hime_win_color_bg;
#if TRAY_ENABLED
int hime_status_tray;
#endif

int pho_hide_row2, pho_in_row1;
#if 0
int gcb_enabled, gcb_position, gcb_position_x, gcb_position_y;
#endif
int hime_bell_volume;
int hime_sound_play_overlap, hime_enable_ctrl_alt_switch;
char *pho_kbm_name, *pho_selkey;
int pho_candicate_col_N, pho_candicate_R2L;


int get_hime_conf_int(char *name, int default_value);

void load_setttings()
{
  hime_font_size = get_hime_conf_int(HIME_FONT_SIZE, 16);
#if UNIX || 1
  get_hime_conf_str(HIME_FONT_NAME, &hime_font_name, "Sans");
#else
  get_hime_conf_str(HIME_FONT_NAME, &hime_font_name, "MingLiU Bold");
#endif
  hime_font_size_tsin_presel = get_hime_conf_int(HIME_FONT_SIZE_TSIN_PRESEL, 16);
  hime_font_size_symbol = get_hime_conf_int(HIME_FONT_SIZE_SYMBOL, 12);
  hime_font_size_tsin_pho_in = get_hime_conf_int(HIME_FONT_SIZE_TSIN_PHO_IN, 13);
  hime_font_size_gtab_in = get_hime_conf_int(HIME_FONT_SIZE_GTAB_IN, 10);
  hime_font_size_pho_near = get_hime_conf_int(HIME_FONT_SIZE_PHO_NEAR, 14);
  hime_font_size_win_kbm = get_hime_conf_int(HIME_FONT_SIZE_WIN_KBM, 10);
  hime_font_size_win_kbm_en = get_hime_conf_int(HIME_FONT_SIZE_WIN_KBM_EN, 8);
  hime_input_style = get_hime_conf_int(HIME_INPUT_STYLE, InputStyleOverSpot);
  hime_root_x = get_hime_conf_int(HIME_ROOT_X, 1600);
  hime_root_y = get_hime_conf_int(HIME_ROOT_Y, 1200);
  hime_pop_up_win = get_hime_conf_int(HIME_POP_UP_WIN, 0);
  hime_inner_frame = get_hime_conf_int(HIME_INNER_FRAME, 1);
  hime_eng_phrase_enabled = get_hime_conf_int(HIME_ENG_PHRASE_ENABLED, 1);
  hime_tray_hf_win_kbm = get_hime_conf_int(HIME_TRAY_HF_WIN_KBM, 0);
#if UNIX
  hime_init_im_enabled = get_hime_conf_int(HIME_INIT_IM_ENABLED, 0);
#else
  hime_init_im_enabled = true;
#endif

  hime_single_state = get_hime_conf_int(HIME_SINGLE_STATE, 0);

  get_hime_conf_str(HIME_STR_IM_CYCLE, &hime_str_im_cycle, "1234567890-=[]\\");
  hime_remote_client = get_hime_conf_int(HIME_REMOTE_CLIENT, 0);
  hime_shift_space_eng_full = get_hime_conf_int(HIME_SHIFT_SPACE_ENG_FULL, 1);
  hime_capslock_lower = get_hime_conf_int(HIME_CAPSLOCK_LOWER, 1);

  get_hime_conf_str(DEFAULT_INPUT_METHOD, &default_input_method_str, "6");
  left_right_button_tips = get_hime_conf_int(LEFT_RIGHT_BUTTON_TIPS, 1);
  hime_im_toggle_keys = get_hime_conf_int(HIME_IM_TOGGLE_KEYS, 0);
#if TRAY_ENABLED
  hime_status_tray = get_hime_conf_int(HIME_STATUS_TRAY, 1);
#endif
  hime_win_sym_click_close = get_hime_conf_int(HIME_WIN_SYM_CLICK_CLOSE, 1);
#if WIN32
  hime_win32_icon = 1;
#else
  hime_win32_icon = get_hime_conf_int(HIME_WIN32_ICON, 1);
#endif

  gtab_dup_select_bell = get_hime_conf_int(GTAB_DUP_SELECT_BELL, 0);
  gtab_space_auto_first = get_hime_conf_int(GTAB_SPACE_AUTO_FIRST, GTAB_space_auto_first_none);
  gtab_auto_select_by_phrase = get_hime_conf_int(GTAB_AUTO_SELECT_BY_PHRASE, GTAB_AUTO_SELECT_BY_PHRASE_AUTO);
  gtab_pre_select = get_hime_conf_int(GTAB_PRE_SELECT, 1);
  gtab_press_full_auto_send = get_hime_conf_int(GTAB_PRESS_FULL_AUTO_SEND, 0);
  gtab_disp_partial_match = get_hime_conf_int(GTAB_DISP_PARTIAL_MATCH, 0);
  gtab_disp_key_codes = get_hime_conf_int(GTAB_DISP_KEY_CODES, 0);
  gtab_disp_im_name = get_hime_conf_int(GTAB_DISP_IM_NAME, 1);
  gtab_invalid_key_in = get_hime_conf_int(GTAB_INVALID_KEY_IN, 0);
  gtab_shift_phrase_key = get_hime_conf_int(GTAB_SHIFT_PHRASE_KEY, 0);
  gtab_hide_row2 = get_hime_conf_int(GTAB_HIDE_ROW2, 0);
  gtab_in_row1 = get_hime_conf_int(GTAB_IN_ROW1, 0);
  gtab_vertical_select = get_hime_conf_int(GTAB_VERTICAL_SELECT, 0);
  gtab_unique_auto_send = get_hime_conf_int(GTAB_UNIQUE_AUTO_SEND, 0);
  gtab_que_wild_card = get_hime_conf_int(GTAB_QUE_WILD_CARD, 0);
  gtab_phrase_pre_select = get_hime_conf_int(GTAB_PHRASE_PRE_SELECT, 1);
  gtab_in_area_button = get_hime_conf_int(GTAB_IN_AREA_BUTTON, 0);

  tsin_phrase_pre_select = get_hime_conf_int(TSIN_PHRASE_PRE_SELECT, 1);
  tsin_chinese_english_toggle_key = get_hime_conf_int(TSIN_CHINESE_ENGLISH_TOGGLE_KEY,
                                    TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Shift);
  tsin_tone_char_input = get_hime_conf_int(TSIN_TONE_CHAR_INPUT, 0);

  tsin_space_opt = get_hime_conf_int(TSIN_SPACE_OPT, TSIN_SPACE_OPT_SELECT_CHAR);
  tsin_buffer_size = get_hime_conf_int(TSIN_BUFFER_SIZE, 40);
  tsin_tab_phrase_end = get_hime_conf_int(TSIN_TAB_PHRASE_END, 1);
  tsin_tail_select_key = get_hime_conf_int(TSIN_TAIL_SELECT_KEY, 0);
  tsin_buffer_editing_mode = get_hime_conf_int(TSIN_BUFFER_EDITING_MODE, 1);
  tsin_use_pho_near = get_hime_conf_int(TSIN_USE_PHO_NEAR, 1);

  phonetic_char_dynamic_sequence = get_hime_conf_int(PHONETIC_CHAR_DYNAMIC_SEQUENCE, 1);
  phonetic_huge_tab = get_hime_conf_int(PHONETIC_HUGE_TAB, 0);
  phonetic_speak = get_hime_conf_int(PHONETIC_SPEAK, 0);
  get_hime_conf_str(PHONETIC_SPEAK_SEL, &phonetic_speak_sel, "3.ogg");

  pho_hide_row2 = get_hime_conf_int(PHO_HIDE_ROW2, 0);
  pho_in_row1 = get_hime_conf_int(PHO_IN_ROW1, 1);

  get_hime_conf_str(TSIN_PHRASE_LINE_COLOR, &tsin_phrase_line_color, "blue");
  get_hime_conf_str(TSIN_CURSOR_COLOR, &tsin_cursor_color, "blue");
  get_hime_conf_str(HIME_SEL_KEY_COLOR, &hime_sel_key_color, "blue");

  if (eng_color_full_str) {
    g_free(eng_color_full_str);
    g_free(cht_color_full_str);
  }

  eng_color_full_str = g_strdup_printf("<span foreground=\"%s\">%s</span>", hime_sel_key_color, _(eng_full_str));
  cht_color_full_str = g_strdup_printf("<span foreground=\"%s\">%s</span>", hime_sel_key_color, _(cht_full_str));

  get_hime_conf_str(HIME_WIN_COLOR_FG, &hime_win_color_fg, "black");
  get_hime_conf_str(HIME_WIN_COLOR_BG, &hime_win_color_bg, "#DADCD5");
  hime_win_color_use = get_hime_conf_int(HIME_WIN_COLOR_USE, 0);
  hime_bell_off = get_hime_conf_int(HIME_BELL_OFF, 0);


#if 0
  gcb_enabled = get_hime_conf_int(GCB_ENABLED, 0);
  gcb_position = get_hime_conf_int(GCB_POSITION, 4);
  gcb_position_x = get_hime_conf_int(GCB_POSITION_X, 0);
  gcb_position_y = get_hime_conf_int(GCB_POSITION_Y, 0);
#endif
  hime_bell_volume = get_hime_conf_int(HIME_BELL_VOLUME, -97);
  hime_sound_play_overlap = get_hime_conf_int(HIME_SOUND_PLAY_OVERLAP, 0);
  hime_enable_ctrl_alt_switch = get_hime_conf_int(HIME_ENABLE_CTRL_ALT_SWITCH, 1);
#if 0
  hime_edit_display = get_hime_conf_int(HIME_EDIT_DISPLAY, HIME_EDIT_DISPLAY_BOTH);
#elif 0
  hime_edit_display = get_hime_conf_int(HIME_EDIT_DISPLAY, HIME_EDIT_DISPLAY_ON_THE_SPOT);
#else
  hime_edit_display = get_hime_conf_int(HIME_EDIT_DISPLAY, HIME_EDIT_DISPLAY_OVER_THE_SPOT);
#endif

  hime_on_the_spot_key = get_hime_conf_int(HIME_ON_THE_SPOT_KEY, 0);
  if (hime_on_the_spot_key)
    hime_edit_display = HIME_EDIT_DISPLAY_ON_THE_SPOT;


  char phokbm[MAX_HIME_STR];

#if DEBUG
  char *hime_pho_kbm = getenv("HIME_PHO_KBM");
  if (hime_pho_kbm)
    strcpy(phokbm, hime_pho_kbm);
  else
#endif
  {
    char *kbm_str = "zo 123456789 1 0";
    get_hime_conf_fstr(PHONETIC_KEYBOARD, phokbm, kbm_str);
  }

  char phokbm_name[32], selkey[32];
  pho_candicate_col_N=0; pho_candicate_R2L=0;
  sscanf(phokbm, "%s %s %d %d",phokbm_name, selkey, &pho_candicate_col_N, &pho_candicate_R2L);

  if (pho_candicate_col_N <= 0)
    pho_candicate_col_N = 1;
  if (pho_candicate_col_N > strlen(selkey))
    pho_candicate_col_N =strlen(selkey);

  if (pho_candicate_R2L<0 || pho_candicate_R2L>1)
    pho_candicate_R2L = 0;

  if (pho_selkey)
    free(pho_selkey);
  pho_selkey = strdup(selkey);

  if (pho_kbm_name)
    free(pho_kbm_name);
  pho_kbm_name = strdup(phokbm_name);
}
