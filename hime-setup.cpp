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
#include "config.h"
#if UNIX
#include <signal.h>
#endif
#if HIME_i18n_message
#include <libintl.h>
#endif
#include "lang.h"

extern gboolean is_chs;

#if UNIX
char utf8_edit[]=HIME_SCRIPT_DIR"/utf8-edit";
#endif

static GtkWidget *check_button_root_style_use,
                 *check_button_hime_pop_up_win,
                 *check_button_hime_inner_frame,
#if TRAY_ENABLED
                 *check_button_hime_status_tray,
                 *check_button_hime_win32_icon,
                 *check_button_hime_tray_hf_win_kbm,
#endif
                 *check_button_hime_win_color_use,
                 *check_button_hime_on_the_spot_key;


static GtkWidget *hime_kbm_window = NULL, *hime_appearance_conf_window;
static GtkClipboard *pclipboard;
static GtkWidget *opt_hime_edit_display;
GtkWidget *main_window;
static GdkColor hime_win_gcolor_fg, hime_win_gcolor_bg, hime_sel_key_gcolor;
gboolean button_order;


typedef struct {
  GdkColor *color;
  char **color_str;
  GtkWidget *color_selector;
  unich_t *title;
} COLORSEL;

COLORSEL colorsel[2] =
  { {&hime_win_gcolor_fg, &hime_win_color_fg, NULL, N_(_L("前景顏色"))},
    {&hime_win_gcolor_bg, &hime_win_color_bg, NULL, N_(_L("背景顏色"))}
  };

struct {
  unich_t *keystr;
  int keynum;
} edit_disp[] = {
  {N_(_L("hime視窗")), HIME_EDIT_DISPLAY_OVER_THE_SPOT},
  {N_(_L("應用程式編輯區")), HIME_EDIT_DISPLAY_ON_THE_SPOT},
  {N_(_L("同時顯示")),  HIME_EDIT_DISPLAY_BOTH},
  { NULL, 0},
};

static gboolean close_application( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  exit(0);
}


void create_kbm_window();

static void cb_kbm()
{
  create_kbm_window();
}

static void cb_hime_tslearn()
{
#if UNIX
  system("hime-tslearn &");
#else
  win32exec("hime-tslearn");
#endif
  exit(0);
}


static void cb_ret(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_destroy((GtkWidget*)user_data);
}

#include <string.h>

static void create_result_win(int res, char *cmd)
{
  char tt[512];

  if (res) {
    sprintf(tt, "%s code:%d '%s'\n%s", _(_L("結果失敗")), res, strerror(res), cmd);
  }
  else
    strcpy(tt, _(_L("結果成功")));

  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_MOUSE);
  gtk_window_set_has_resize_grip(GTK_WINDOW(main_window), FALSE);

  GtkWidget *button = gtk_button_new_with_label(tt);
  gtk_container_add (GTK_CONTAINER (main_window), button);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_ret), main_window);

  gtk_widget_show_all(main_window);
}


static void cb_ts_export()
{
   GtkWidget *file_selector;
   if (button_order)
       file_selector = gtk_file_chooser_dialog_new(_(_L("請輸入要匯出的檔案名稱")),
                              GTK_WINDOW(main_window),
                              GTK_FILE_CHOOSER_ACTION_OPEN,
                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              NULL);
   else
       file_selector = gtk_file_chooser_dialog_new(_(_L("請輸入要匯出的檔案名稱")),
                              GTK_WINDOW(main_window),
                              GTK_FILE_CHOOSER_ACTION_OPEN,
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                              NULL);
   if (gtk_dialog_run (GTK_DIALOG (file_selector)) == GTK_RESPONSE_ACCEPT) {
       gchar *selected_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_selector));
       char hime_dir[512];
       get_hime_dir(hime_dir);
       char cmd[512];
       char fname[256];
       char *filename=inmd[default_input_method].filename;

       if (inmd[default_input_method].method_type==method_type_TSIN)
         get_hime_user_fname(tsin32_f, fname);
       else
       if (filename) {
         char tt[256];
         strcat(strcpy(tt, filename), ".append.gtab.tsin-db");
         if (!get_hime_user_fname(tt, fname)) {
           strcat(strcpy(tt, filename), ".tsin-db");
           if (!get_hime_user_fname(tt, fname))
             p_err("cannot find %s", fname);
         }
       }
#if UNIX
       snprintf(cmd, sizeof(cmd), HIME_BIN_DIR"/hime-tsd2a32 %s -o %s", fname, selected_filename);
       dbg("exec %s\n", cmd);
       int res = system(cmd);
       res = 0; // some problem in system(), the exit code is not reliable
       create_result_win(res, cmd);
#else
	   char para[256];
       sprintf_s(para, sizeof(para), "\"%s\" -o \"%s\"", fname, selected_filename);
	   win32exec_para("hime-tsd2a32", para);
#endif
   }
   gtk_widget_destroy (file_selector);
}

static void ts_import(const gchar *selected_filename)
{
   char cmd[256];
#if UNIX
   if (inmd[default_input_method].method_type==method_type_TSIN) {
     snprintf(cmd, sizeof(cmd),
        "cd %s/.config/hime && "HIME_BIN_DIR"/hime-tsd2a32 %s > tmpfile && cat %s >> tmpfile && "HIME_BIN_DIR"/hime-tsa2d32 tmpfile %s",
        getenv("HOME"), tsin32_f, selected_filename, tsin32_f);
     int res = system(cmd);
     res = 0;
     create_result_win(res, cmd);
   } else {
     char tt[512];
     sprintf(tt, HIME_SCRIPT_DIR"/tsin-gtab-import %s '%s'", inmd[default_input_method].filename,
     selected_filename);
     system(tt);
   }
#else
   if (inmd[default_input_method].method_type==method_type_TSIN)
     win32exec_script_va("ts-import.bat", (char *)selected_filename, tsin32_f, NULL);
   else {
     win32exec_script_va("ts-gtab-import.bat", inmd[default_input_method].filename,  selected_filename, NULL);
   }
#endif
}

#if !GTK_CHECK_VERSION(2,4,0)
static void cb_file_ts_import(GtkWidget *widget, gpointer user_data)
{
   GtkWidget *file_selector = (GtkWidget *)user_data;
   const gchar *selected_filename;

   selected_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_selector));
//   g_print ("Selected filename: %s\n", selected_filename);

   ts_import(selected_filename);
}
#endif

static void cb_ts_import()
{
   /* Create the selector */

#if GTK_CHECK_VERSION(2,4,0)
   GtkWidget *file_selector;
   if (button_order)
       file_selector = gtk_file_chooser_dialog_new(_(_L("請輸入要匯入的檔案名稱")),
                              GTK_WINDOW(main_window),
                              GTK_FILE_CHOOSER_ACTION_OPEN,
                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              NULL);
   else
       file_selector = gtk_file_chooser_dialog_new(_(_L("請輸入要匯入的檔案名稱")),
                              GTK_WINDOW(main_window),
                              GTK_FILE_CHOOSER_ACTION_OPEN,
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                              NULL);
   if (gtk_dialog_run (GTK_DIALOG (file_selector)) == GTK_RESPONSE_ACCEPT) {
       gchar *selected_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_selector));

       ts_import(selected_filename);
   }
   gtk_widget_destroy (file_selector);
#else
   GtkWidget *file_selector = gtk_file_selection_new (_("請輸入要匯入的檔案名稱"));

   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (cb_file_ts_import),
                     (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   gtk_widget_show(file_selector);
#endif
}

static void cb_ts_edit()
{
#if 0
#if UNIX
  if (inmd[default_input_method].method_type==method_type_TSIN) {
    char tt[512];
    sprintf(tt, "( cd ~/.config/hime && "HIME_BIN_DIR"/hime-tsd2a32 %s > tmpfile && %s tmpfile && "HIME_BIN_DIR"/hime-tsa2d32 tmpfile %s) &",
      tsin32_f, utf8_edit, tsin32_f);
    dbg("exec %s\n", tt);
    system(tt);
  } else {
    char tt[512];
    sprintf(tt, HIME_SCRIPT_DIR"/tsin-gtab-edit %s", inmd[default_input_method].filename);
    system(tt);
  }
#else
  if (inmd[default_input_method].method_type==method_type_TSIN)
    win32exec_script("hime-ts-edit.bat", tsin32_f);
  else {
    win32exec_script("ts-gtab-edit.bat", inmd[default_input_method].filename);
  }
#endif
#else
#if UNIX
  system(HIME_BIN_DIR"/hime-ts-edit");
#else
  win32exec("hime-ts-edit.exe");
#endif
#endif
}


static void cb_ts_import_sys()
{
#if UNIX
  char tt[512];
  sprintf(tt, "cd ~/.config/hime && "HIME_BIN_DIR"/hime-tsd2a32 %s > tmpfile && "HIME_BIN_DIR"/hime-tsd2a32 %s/%s >> tmpfile && "HIME_BIN_DIR"/hime-tsa2d32 tmpfile",
    tsin32_f, HIME_TABLE_DIR, tsin32_f);
  dbg("exec %s\n", tt);
  system(tt);
#else
  win32exec_script("ts-import-sys.bat", tsin32_f);
#endif
}


static void cb_alt_shift()
{
#if UNIX
  char tt[512];
  sprintf(tt, "( cd ~/.config/hime && %s phrase.table ) &", utf8_edit);
  system(tt);
#else
  char fname[512];
  get_hime_user_fname("phrase.table", fname);
  win32exec_script("utf8-edit.bat", fname);
#endif
}


static void cb_symbol_table()
{
  char tt[512];
#if UNIX
  sprintf(tt, "( cd ~/.config/hime && %s symbol-table ) &", utf8_edit);
  system(tt);
#else
  char fname[512];
  get_hime_user_fname("symbol-table", fname);
  win32exec_script("utf8-edit.bat", fname);
#endif
}


int html_browser(char *fname);

static void cb_help()
{
#if UNIX
  html_browser(DOC_DIR"/README.html");
#else
  char fname[512];
  strcpy(fname, hime_program_files_path);
  strcat(fname, "\\README.html");
  html_browser(fname);
#endif
}

static GtkWidget *spinner_hime_font_size, *spinner_hime_font_size_tsin_presel,
                 *spinner_hime_font_size_symbol,*spinner_hime_font_size_pho_near,
                 *spinner_hime_font_size_win_kbm,
                 *spinner_hime_font_size_win_kbm_en,
                 *spinner_hime_font_size_tsin_pho_in, *spinner_hime_font_size_gtab_in, *spinner_root_style_x,
                 *spinner_root_style_y, *font_sel;

static GtkWidget *label_win_color_test, *event_box_win_color_test;

static gboolean cb_appearance_conf_ok( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  int font_size = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_hime_font_size));
  save_hime_conf_int(HIME_FONT_SIZE, font_size);

#if GTK_CHECK_VERSION(2,4,0)
  char fname[128];
  strcpy(fname, gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_sel)));
  int len = strlen(fname)-1;

  while (len > 0 && isdigit(fname[len])) {
       fname[len--]=0;
  }

  while (len > 0 && fname[len]==' ') {
       fname[len--]=0;
  }

  save_hime_conf_str(HIME_FONT_NAME, fname);
#endif

  int font_size_tsin_presel = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_hime_font_size_tsin_presel));
  save_hime_conf_int(HIME_FONT_SIZE_TSIN_PRESEL, font_size_tsin_presel);

  int font_size_symbol = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_hime_font_size_symbol));
  save_hime_conf_int(HIME_FONT_SIZE_SYMBOL, font_size_symbol);

  int font_size_tsin_pho_in = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_hime_font_size_tsin_pho_in));
  save_hime_conf_int(HIME_FONT_SIZE_TSIN_PHO_IN, font_size_tsin_pho_in);

  int font_size_pho_near = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_hime_font_size_pho_near));
  save_hime_conf_int(HIME_FONT_SIZE_PHO_NEAR, font_size_pho_near);

  int font_size_gtab_in = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_hime_font_size_gtab_in));
  save_hime_conf_int(HIME_FONT_SIZE_GTAB_IN, font_size_gtab_in);

  int font_size_win_kbm = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_hime_font_size_win_kbm));
  save_hime_conf_int(HIME_FONT_SIZE_WIN_KBM, font_size_win_kbm);
  int font_size_win_kbm_en = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_hime_font_size_win_kbm_en));
  save_hime_conf_int(HIME_FONT_SIZE_WIN_KBM_EN, font_size_win_kbm_en);

  int hime_pop_up_win = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_pop_up_win));
  save_hime_conf_int(HIME_POP_UP_WIN, hime_pop_up_win);

  int hime_root_x = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_root_style_x));
  save_hime_conf_int(HIME_ROOT_X, hime_root_x);

  int hime_root_y = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_root_style_y));
  save_hime_conf_int(HIME_ROOT_Y, hime_root_y);

  int style = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_root_style_use)) ?
            InputStyleRoot : InputStyleOverSpot;
  save_hime_conf_int(HIME_INPUT_STYLE, style);

  save_hime_conf_int(HIME_INNER_FRAME, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_inner_frame)));
#if TRAY_ENABLED
  save_hime_conf_int(HIME_STATUS_TRAY, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_status_tray)));
  save_hime_conf_int(HIME_WIN32_ICON, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_win32_icon)));
#endif

  gchar *cstr = gtk_color_selection_palette_to_string(&hime_win_gcolor_fg, 1);
  dbg("color fg %s\n", cstr);
  save_hime_conf_str(HIME_WIN_COLOR_FG, cstr);
  g_free(cstr);

  cstr = gtk_color_selection_palette_to_string(&hime_win_gcolor_bg, 1);
  dbg("color bg %s\n", cstr);
  save_hime_conf_str(HIME_WIN_COLOR_BG, cstr);
  g_free(cstr);

  save_hime_conf_int(HIME_WIN_COLOR_USE, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_win_color_use)));
  save_hime_conf_int(HIME_ON_THE_SPOT_KEY, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_on_the_spot_key)));
  save_hime_conf_int(HIME_TRAY_HF_WIN_KBM, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_tray_hf_win_kbm)));

  cstr = gtk_color_selection_palette_to_string(&hime_sel_key_gcolor, 1);
  dbg("selkey color %s\n", cstr);
  save_hime_conf_str(HIME_SEL_KEY_COLOR, cstr);

  int idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_hime_edit_display));
  save_hime_conf_int(HIME_EDIT_DISPLAY, edit_disp[idx].keynum);

  g_free(cstr);


  send_hime_message(
#if UNIX
	  GDK_DISPLAY(),
#endif
	  CHANGE_FONT_SIZE);
  gtk_widget_destroy(hime_appearance_conf_window); hime_appearance_conf_window = NULL;

  return TRUE;
}

static gboolean close_appearance_conf_window( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  gtk_widget_destroy(hime_appearance_conf_window); hime_appearance_conf_window = NULL;
  return TRUE;
}


static void cb_savecb_hime_win_color_fg(GtkWidget *widget, gpointer user_data)
{
  COLORSEL *sel = (COLORSEL *)user_data;
  GtkWidget *color_selector = sel->color_selector;
  GdkColor *col = sel->color;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(color_selector))), col);

  if (sel->color == &hime_win_gcolor_fg) {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, col);
#else
    GdkRGBA rgbfg;
    gdk_rgba_parse(&rgbfg, gdk_color_to_string(col));
    gtk_widget_override_color(label_win_color_test, GTK_STATE_FLAG_NORMAL, &rgbfg);
#endif
  } else {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, col);
#else
    GdkRGBA rgbbg;
    gdk_rgba_parse(&rgbbg, gdk_color_to_string(col));
    gtk_widget_override_background_color(event_box_win_color_test, GTK_STATE_FLAG_NORMAL, &rgbbg);
#endif
  }
}

static gboolean cb_hime_win_color_fg( GtkWidget *widget,
                                   gpointer   data)
{
  COLORSEL *sel = (COLORSEL *)data;
  GtkWidget *color_selector = gtk_color_selection_dialog_new (_(sel->title));

  gdk_color_parse(*sel->color_str, sel->color);

  gtk_color_selection_set_current_color(
          GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(color_selector))),
          sel->color);


  sel->color_selector = color_selector;

  gtk_widget_show((GtkWidget*)color_selector);
#if 1
  if (gtk_dialog_run(GTK_DIALOG(color_selector)) == GTK_RESPONSE_OK)
    cb_savecb_hime_win_color_fg((GtkWidget *)color_selector, (gpointer) sel);
  gtk_widget_destroy(color_selector);
#endif
  return TRUE;
}

void disp_fg_bg_color()
{
  dbg("disp_fg_bg_color\n");
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(check_button_hime_win_color_use))) {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, &hime_win_gcolor_fg);
    dbg("hime_win_gcolor_bg %d\n", hime_win_gcolor_bg);
    gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, &hime_win_gcolor_bg);
#else
    GdkRGBA rgbfg, rgbbg;
    gdk_rgba_parse(&rgbfg, gdk_color_to_string(&hime_win_gcolor_fg));
    gdk_rgba_parse(&rgbbg, gdk_color_to_string(&hime_win_gcolor_bg));
    gtk_widget_override_color(label_win_color_test, GTK_STATE_FLAG_NORMAL, &rgbfg);
    gtk_widget_override_background_color(event_box_win_color_test, GTK_STATE_FLAG_NORMAL, &rgbbg);
#endif
  } else {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, NULL);
    gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, NULL);
#else
    gtk_widget_override_color(label_win_color_test, GTK_STATE_FLAG_NORMAL, NULL);
    gtk_widget_override_background_color(event_box_win_color_test, GTK_STATE_FLAG_NORMAL, NULL);
#endif
  }

  char *key_color = gtk_color_selection_palette_to_string(&hime_sel_key_gcolor, 1);
  unich_t tt[512];
#if UNIX
#if PANGO_VERSION_CHECK(1,22,0)
  sprintf
(tt, _(_L("<span foreground=\"%s\" font=\"%d\">7</span><span font=\"%d\">測試</span>")), key_color,
hime_font_size_tsin_presel, hime_font_size_tsin_presel);
#else
  sprintf
(tt, _(_L("<span foreground=\"%s\" font_desc=\"%d\">7</span><span font_desc=\"%d\">測試</span>")), key_color,
hime_font_size_tsin_presel, hime_font_size_tsin_presel);
#endif
#else
  swprintf
(tt, _L("<span foreground=\"%S\" font=\"%d\">7</span><span font=\"%d\">測試</span>"), key_color, hime_font_size_tsin_presel, hime_font_size_tsin_presel);
#endif

  gtk_label_set_markup(GTK_LABEL(label_win_color_test), _(tt));
}

static void cb_save_hime_sel_key_color(GtkWidget *widget, gpointer user_data)
{
  GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)user_data;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(color_selector)), &hime_sel_key_gcolor);
  hime_sel_key_color = gtk_color_selection_palette_to_string(&hime_sel_key_gcolor, 1);

  g_snprintf(eng_color_full_str, 128, "<span foreground=\"%s\">%s</span>", hime_sel_key_color, _(eng_full_str));
  g_snprintf(cht_color_full_str, 128, "<span foreground=\"%s\">%s</span>", hime_sel_key_color, _(cht_full_str));

  disp_fg_bg_color();
}


static gboolean cb_hime_sel_key_color( GtkWidget *widget, gpointer data)
{
   GtkWidget *color_selector = gtk_color_selection_dialog_new (_(_L("選擇鍵的顏色")));

   gtk_color_selection_set_current_color(
           GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(color_selector))),
           &hime_sel_key_gcolor);


   gtk_widget_show(color_selector);
   if (gtk_dialog_run(GTK_DIALOG(color_selector)) == GTK_RESPONSE_OK)
     cb_save_hime_sel_key_color(color_selector, (gpointer) color_selector);
   gtk_widget_destroy(color_selector);

   return TRUE;
}

void cb_button_hime_on_the_spot_key(GtkToggleButton *togglebutton, gpointer user_data)
{
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(togglebutton)))
    return;
  int i;
  for (i=0; edit_disp[i].keystr; i++)
   if (edit_disp[i].keynum == HIME_EDIT_DISPLAY_ON_THE_SPOT)
     gtk_combo_box_set_active (GTK_COMBO_BOX (opt_hime_edit_display), i);
}

void combo_selected(GtkWidget *widget, gpointer window)
{
  int idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_hime_edit_display));
  if (edit_disp[idx].keynum !=  HIME_EDIT_DISPLAY_ON_THE_SPOT) {
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check_button_hime_on_the_spot_key), FALSE);
  }
}

static GtkWidget *create_hime_edit_display()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);
  GtkWidget *label = gtk_label_new(_(_L("編輯區顯示")));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_hime_edit_display = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
  GtkWidget *menu = gtk_menu_new ();
#endif
  gtk_box_pack_start (GTK_BOX (hbox), opt_hime_edit_display, FALSE, FALSE, 0);

  int i, current_idx=0;

  for(i=0; edit_disp[i].keystr; i++) {
#if !GTK_CHECK_VERSION(2,4,0)
    GtkWidget *item = gtk_menu_item_new_with_label (_(edit_disp[i].keystr));
#endif

    if (edit_disp[i].keynum == hime_edit_display)
      current_idx = i;

#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_hime_edit_display), _(edit_disp[i].keystr));
#else
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
#endif
  }

#if !GTK_CHECK_VERSION(2,4,0)
  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_hime_edit_display), menu);
#endif
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_hime_edit_display), current_idx);

  check_button_hime_on_the_spot_key = gtk_check_button_new_with_label (_(_L("顯示字根於應用程式中\n(OnTheSpot)")));
  g_signal_connect (G_OBJECT (check_button_hime_on_the_spot_key), "toggled",
                    G_CALLBACK (cb_button_hime_on_the_spot_key), NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_on_the_spot_key),
       hime_on_the_spot_key);
  gtk_box_pack_start (GTK_BOX (hbox), check_button_hime_on_the_spot_key, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(opt_hime_edit_display), "changed",
        G_CALLBACK(combo_selected), (gpointer) NULL);

  return hbox;
}



static gboolean cb_hime_win_color_use(GtkToggleButton *togglebutton, gpointer user_data)
{
  dbg("cb_hime_win_color_use\n");
  disp_fg_bg_color();
  return TRUE;
}


void create_appearance_conf_window()
{
  if (hime_appearance_conf_window) {
    gtk_window_present(GTK_WINDOW(hime_appearance_conf_window));
    return;
  }

  load_setttings();

  hime_appearance_conf_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(hime_appearance_conf_window), GTK_WIN_POS_MOUSE);


  gtk_window_set_has_resize_grip(GTK_WINDOW(hime_appearance_conf_window), FALSE);

  g_signal_connect (G_OBJECT (hime_appearance_conf_window), "delete_event",
                    G_CALLBACK (close_appearance_conf_window),
                    NULL);

  gtk_window_set_title (GTK_WINDOW (hime_appearance_conf_window), _(_L("輸入視窗外觀設定")));
  gtk_container_set_border_width (GTK_CONTAINER (hime_appearance_conf_window), 3);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_top), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (hime_appearance_conf_window), vbox_top);

  GtkWidget *hbox_hime_font_size = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size = gtk_label_new(_(_L("字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size), label_hime_font_size, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size = gtk_spin_button_new (adj_hime_font_size, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size), spinner_hime_font_size, FALSE, FALSE, 0);


  GtkWidget *hbox_hime_font_size_symbol = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_symbol, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_symbol = gtk_label_new(_(_L("符號選擇視窗字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_symbol), label_hime_font_size_symbol, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_symbol =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_symbol, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_symbol = gtk_spin_button_new (adj_hime_font_size_symbol, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_symbol), spinner_hime_font_size_symbol, FALSE, FALSE, 0);


  GtkWidget *hbox_hime_font_size_tsin_presel = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_tsin_presel, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_tsin_presel = gtk_label_new(_(_L("詞音&gtab預選詞視窗字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_tsin_presel), label_hime_font_size_tsin_presel, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_tsin_presel =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_tsin_presel, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_tsin_presel = gtk_spin_button_new (adj_hime_font_size_tsin_presel, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_tsin_presel), spinner_hime_font_size_tsin_presel, FALSE, FALSE, 0);


  GtkWidget *hbox_hime_font_size_tsin_pho_in = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_tsin_pho_in, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_tsin_pho_in = gtk_label_new(_(_L("詞音注音輸入區字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_tsin_pho_in), label_hime_font_size_tsin_pho_in, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_tsin_pho_in =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_tsin_pho_in, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_tsin_pho_in = gtk_spin_button_new (adj_hime_font_size_tsin_pho_in, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_tsin_pho_in), spinner_hime_font_size_tsin_pho_in, FALSE, FALSE, 0);


  GtkWidget *hbox_hime_font_size_pho_near = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_pho_near, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_pho_near = gtk_label_new(_(_L("詞音近似音顯示字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_pho_near), label_hime_font_size_pho_near, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_pho_near =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_pho_near, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_pho_near = gtk_spin_button_new (adj_hime_font_size_pho_near, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_pho_near), spinner_hime_font_size_pho_near, FALSE, FALSE, 0);


  GtkWidget *hbox_hime_font_size_gtab_in = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_gtab_in, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_gtab_in = gtk_label_new(_(_L("gtab(倉頡…)輸入區字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_gtab_in), label_hime_font_size_gtab_in, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_gtab_in =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_gtab_in, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_gtab_in = gtk_spin_button_new (adj_hime_font_size_gtab_in, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_gtab_in), spinner_hime_font_size_gtab_in, FALSE, FALSE, 0);

  GtkWidget *hbox_hime_font_size_win_kbm = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_win_kbm, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_win_kbm = gtk_label_new(_(_L("小鍵盤字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_win_kbm), label_hime_font_size_win_kbm, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_win_kbm =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_win_kbm, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_win_kbm = gtk_spin_button_new (adj_hime_font_size_win_kbm, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_win_kbm), spinner_hime_font_size_win_kbm, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_win_kbm_en = gtk_label_new(_(_L("英數")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_win_kbm), label_hime_font_size_win_kbm_en, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_win_kbm_en =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_win_kbm_en, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_win_kbm_en = gtk_spin_button_new (adj_hime_font_size_win_kbm_en, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_win_kbm), spinner_hime_font_size_win_kbm_en, FALSE, FALSE, 0);


#if GTK_CHECK_VERSION(2,4,0)
  char tt[128];
  sprintf(tt, "%s %d", hime_font_name, hime_font_size);
  font_sel = gtk_font_button_new_with_font (tt);
  gtk_box_pack_start (GTK_BOX (vbox_top), font_sel, FALSE, FALSE, 0);
#endif

  GtkWidget *hbox_hime_pop_up_win = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_top), hbox_hime_pop_up_win, FALSE, FALSE, 0);
  check_button_hime_pop_up_win = gtk_check_button_new_with_label (_(_L("在有輸入字根時才會彈出輸入視窗")));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_pop_up_win),
       hime_pop_up_win);
  gtk_box_pack_start (GTK_BOX(hbox_hime_pop_up_win), check_button_hime_pop_up_win, FALSE, FALSE, 0);

  GtkWidget *frame_root_style = gtk_frame_new(_(_L("固定 hime 視窗位置")));
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_root_style, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_root_style), 3);
  GtkWidget *vbox_root_style = gtk_vbox_new (FALSE, 10);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_root_style), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (frame_root_style), vbox_root_style);

  GtkWidget *hbox_root_style_use = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_root_style), hbox_root_style_use, FALSE, FALSE, 0);
  check_button_root_style_use = gtk_check_button_new_with_label (_(_L("啟用")));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_root_style_use),
       hime_input_style == InputStyleRoot);
  gtk_box_pack_start (GTK_BOX(hbox_root_style_use), check_button_root_style_use, FALSE, FALSE, 0);


  GtkWidget *hbox_root_style = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_root_style), hbox_root_style, FALSE, FALSE, 0);

  GtkAdjustment *adj_root_style_x =
   (GtkAdjustment *) gtk_adjustment_new (hime_root_x, 0.0, 5120.0, 1.0, 1.0, 0.0);
  spinner_root_style_x = gtk_spin_button_new (adj_root_style_x, 0, 0);
  gtk_widget_set_hexpand (spinner_root_style_x, TRUE);
  gtk_container_add (GTK_CONTAINER (hbox_root_style), spinner_root_style_x);

  GtkAdjustment *adj_root_style_y =
   (GtkAdjustment *) gtk_adjustment_new (hime_root_y, 0.0, 2880.0, 1.0, 1.0, 0.0);
  spinner_root_style_y = gtk_spin_button_new (adj_root_style_y, 0, 0);
  gtk_widget_set_hexpand (spinner_root_style_y, TRUE);
  gtk_container_add (GTK_CONTAINER (hbox_root_style), spinner_root_style_y);


  gtk_box_pack_start (GTK_BOX(vbox_top), create_hime_edit_display(), FALSE, FALSE, 0);

  GtkWidget *hbox_hime_inner_frame = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_top), hbox_hime_inner_frame, FALSE, FALSE, 0);
  check_button_hime_inner_frame = gtk_check_button_new_with_label (_(_L("字根區顯示內框")));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_inner_frame),
       hime_inner_frame);
  gtk_box_pack_start (GTK_BOX(hbox_hime_inner_frame), check_button_hime_inner_frame, FALSE, FALSE, 0);

#if TRAY_ENABLED
  GtkWidget *hbox_hime_status_tray = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_top), hbox_hime_status_tray, FALSE, FALSE, 0);
  check_button_hime_status_tray = gtk_check_button_new_with_label (_(_L("啟用 System Tray Icon")));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_status_tray),
       hime_status_tray);
  gtk_box_pack_start (GTK_BOX(hbox_hime_status_tray), check_button_hime_status_tray, FALSE, FALSE, 0);
#if UNIX
  check_button_hime_win32_icon = gtk_check_button_new_with_label (_(_L("使用雙圖示")));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_win32_icon),
       hime_win32_icon);
  gtk_box_pack_start (GTK_BOX(hbox_hime_status_tray), check_button_hime_win32_icon, FALSE, FALSE, 0);
#endif
  check_button_hime_tray_hf_win_kbm = gtk_check_button_new_with_label (_(_L("全半形左鍵\n切換小鍵盤")));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_tray_hf_win_kbm),
       hime_tray_hf_win_kbm);
  gtk_box_pack_start (GTK_BOX(hbox_hime_status_tray), check_button_hime_tray_hf_win_kbm, FALSE, FALSE, 0);
#endif

  GtkWidget *frame_win_color = gtk_frame_new(_(_L("顏色選擇")));
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_win_color, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_win_color), 1);
  GtkWidget *vbox_win_color = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_win_color), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (frame_win_color), vbox_win_color);

  GtkWidget *hbox_win_color_use = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_win_color), hbox_win_color_use, FALSE, FALSE, 0);
  check_button_hime_win_color_use = gtk_check_button_new_with_label (_(_L("自訂顏色主題")));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_win_color_use),
       hime_win_color_use);

  g_signal_connect (G_OBJECT (check_button_hime_win_color_use), "clicked",
                    G_CALLBACK (cb_hime_win_color_use), NULL);

  gtk_box_pack_start (GTK_BOX(hbox_win_color_use), check_button_hime_win_color_use, FALSE, FALSE, 0);
  event_box_win_color_test = gtk_event_box_new();
// this will make the color test failed
//  gtk_event_box_set_visible_window (GTK_EVENT_BOX(event_box_win_color_test), FALSE);
  gtk_box_pack_start (GTK_BOX(vbox_win_color), event_box_win_color_test, FALSE, FALSE, 0);
  label_win_color_test = gtk_label_new(NULL);
  gtk_container_add (GTK_CONTAINER(event_box_win_color_test), label_win_color_test);
  GtkWidget *hbox_win_color_fbg = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_win_color), hbox_win_color_fbg, FALSE, FALSE, 0);
  GtkWidget *button_fg = gtk_button_new_with_label(_(_L("前景顏色")));
  gtk_widget_set_hexpand (button_fg, TRUE);
  gtk_widget_set_halign (button_fg, GTK_ALIGN_FILL);
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_fg, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_fg), "clicked",
                    G_CALLBACK (cb_hime_win_color_fg), &colorsel[0]);
  gdk_color_parse(hime_win_color_fg, &hime_win_gcolor_fg);
//  gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, &hime_win_gcolor_fg);
  gdk_color_parse(hime_win_color_bg, &hime_win_gcolor_bg);
//  gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, &hime_win_gcolor_bg);

  GtkWidget *button_bg = gtk_button_new_with_label(_(_L("背景顏色")));
  gtk_widget_set_hexpand (button_bg, TRUE);
  gtk_widget_set_halign (button_bg, GTK_ALIGN_FILL);
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_bg, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_bg), "clicked",
                    G_CALLBACK (cb_hime_win_color_fg), &colorsel[1]);

  GtkWidget *button_hime_sel_key_color = gtk_button_new_with_label(_(_L("選擇鍵顏色")));
  gtk_widget_set_hexpand (button_hime_sel_key_color, TRUE);
  gtk_widget_set_halign (button_hime_sel_key_color, GTK_ALIGN_FILL);
  g_signal_connect (G_OBJECT (button_hime_sel_key_color), "clicked",
                    G_CALLBACK (cb_hime_sel_key_color), G_OBJECT (hime_kbm_window));
  gdk_color_parse(hime_sel_key_color, &hime_sel_key_gcolor);
  gtk_container_add (GTK_CONTAINER (hbox_win_color_fbg), button_hime_sel_key_color);

  disp_fg_bg_color();

  GtkWidget *hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
  gtk_grid_set_column_homogeneous(GTK_GRID(hbox_cancel_ok), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_cancel_ok, FALSE, FALSE, 0);

  GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  if (button_order)
    gtk_box_pack_end (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);
  else
    gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT (button_cancel), "clicked",
                            G_CALLBACK (close_appearance_conf_window),
                            G_OBJECT (hime_appearance_conf_window));

  GtkWidget *button_close = gtk_button_new_from_stock (GTK_STOCK_OK);
#if !GTK_CHECK_VERSION(2,91,2)
  if (button_order)
    gtk_box_pack_end (GTK_BOX (hbox_cancel_ok), button_close, TRUE, TRUE, 0);
  else
    gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_close, TRUE, TRUE, 0);
#else
  if (button_order)
    gtk_grid_attach_next_to (GTK_BOX (hbox_cancel_ok), button_close, button_cancel, GTK_POS_LEFT, 1, 1);
  else
    gtk_grid_attach_next_to (GTK_BOX (hbox_cancel_ok), button_close, button_cancel, GTK_POS_RIGHT, 1, 1);
#endif

  g_signal_connect_swapped (G_OBJECT (button_close), "clicked",
                            G_CALLBACK (cb_appearance_conf_ok),
                            G_OBJECT (hime_kbm_window));

  GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button_close);

  gtk_widget_show_all (hime_appearance_conf_window);

  return;
}


static void cb_appearance_conf()
{
  create_appearance_conf_window();
}

void create_gtab_conf_window();

static void cb_gtab_conf()
{
  create_gtab_conf_window();
}


static void cb_gb_output_toggle()
{
  send_hime_message(
#if UNIX
	  GDK_DISPLAY(),
#endif
	  GB_OUTPUT_TOGGLE);
  exit(0);
}

static void cb_gb_translate_toggle()
{
#if WIN32
  win32exec("hime-sim2trad");
#else
  system(HIME_BIN_DIR"/hime-sim2trad &");
#endif
  exit(0);
}


static void cb_juying_learn()
{
#if WIN32
  win32exec("hime-juyin-learn");
#else
  system(HIME_BIN_DIR"/hime-juyin-learn &");
#endif
  exit(0);
}

#if 0
int hime_pid;
static void cb_hime_exit()
{
#if UNIX
  kill(hime_pid, 9);
#else
  TerminateProcess(
#endif
}
#endif

void create_gtablist_window();
static void cb_default_input_method()
{
  create_gtablist_window();
}

void create_about_window();
void set_window_hime_icon(GtkWidget *window);

static gboolean timeout_minimize_main_window(gpointer data)
{
  gtk_window_resize(GTK_WINDOW(main_window), 32, 32);
  return FALSE;
}

static void
expander_callback (GObject    *object,
                   GParamSpec *param_spec,
                   gpointer    user_data)
{
  GtkExpander *expander;
  expander = GTK_EXPANDER (object);

  if (gtk_expander_get_expanded (expander)) {
  } else {
    // cannot do this here
    // gtk_window_resize(GTK_WINDOW(main_window), 32, 32);
    g_timeout_add(200, timeout_minimize_main_window, NULL);
  }
}

#include "pho.h"
#include "tsin.h"
#include "gst.h"
#include "im-client/hime-im-client-attr.h"
#include "win1.h"
#include "hime-module.h"
#include "hime-module-cb.h"
HIME_module_callback_functions *init_HIME_module_callback_functions(char *sofile);

static void create_main_win()
{
  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

  gtk_window_set_has_resize_grip(GTK_WINDOW(main_window), FALSE);

  g_signal_connect (G_OBJECT (main_window), "delete_event",
                     G_CALLBACK (close_application),
                     NULL);

  set_window_hime_icon(main_window);

  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (main_window), vbox);

  GtkWidget *button_default_input_method = gtk_button_new_with_label(_(_L("內定輸入法 & 開啟/關閉")));
  gtk_box_pack_start (GTK_BOX (vbox), button_default_input_method, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_default_input_method), "clicked",
                    G_CALLBACK (cb_default_input_method), NULL);

  GtkWidget *button_appearance_conf = gtk_button_new_with_label(_(_L("外觀設定")));
  gtk_box_pack_start (GTK_BOX (vbox), button_appearance_conf, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_appearance_conf), "clicked",
                    G_CALLBACK (cb_appearance_conf), NULL);

  GtkWidget *button_kbm = gtk_button_new_with_label(_(_L("注音/詞音/拼音設定")));
  gtk_box_pack_start (GTK_BOX (vbox), button_kbm, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_kbm), "clicked",
                    G_CALLBACK (cb_kbm), NULL);

  GtkWidget *button_gtab_conf = gtk_button_new_with_label(_(_L("倉頡/行列/大易設定")));
  gtk_box_pack_start (GTK_BOX (vbox), button_gtab_conf, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_gtab_conf), "clicked",
                    G_CALLBACK (cb_gtab_conf), NULL);

  int i;
  for (i=0; i < inmdN; i++) {
    INMD *pinmd = &inmd[i];
    if (pinmd->method_type != method_type_MODULE || pinmd->disabled)
      continue;

    HIME_module_callback_functions *f = init_HIME_module_callback_functions(pinmd->filename);
    if (!f)
      continue;

    if (!f->module_setup_window_create) {
      free(f);
      continue;
    }

    char tt[128];
    strcpy(tt, pinmd->cname);
    strcat(tt, _(_L("設定")));
    GtkWidget *button_chewing_input_method = gtk_button_new_with_label(tt);
    gtk_box_pack_start (GTK_BOX (vbox), button_chewing_input_method, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (button_chewing_input_method), "clicked",
                    G_CALLBACK (f->module_setup_window_create), NULL);
  }


  GtkWidget *button_alt_shift = gtk_button_new_with_label(_(_L("alt-shift 片語編輯")));
  gtk_box_pack_start (GTK_BOX (vbox), button_alt_shift, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_alt_shift), "clicked",
                    G_CALLBACK (cb_alt_shift), NULL);

  GtkWidget *button_symbol_table = gtk_button_new_with_label(_(_L("符號表編輯")));
  gtk_box_pack_start (GTK_BOX (vbox), button_symbol_table, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_symbol_table), "clicked",
                    G_CALLBACK (cb_symbol_table), NULL);

#if TRAY_ENABLED
  if (!hime_status_tray)
  {
#endif
    GtkWidget *button_gb_output_toggle = gtk_button_new_with_label(_(_L("簡體字輸出切換")));
    gtk_box_pack_start (GTK_BOX (vbox), button_gb_output_toggle, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (button_gb_output_toggle), "clicked",
                      G_CALLBACK (cb_gb_output_toggle), NULL);
#if TRAY_ENABLED
  }
#endif

  GtkWidget *button_gb_translate_toggle = gtk_button_new_with_label(_(_L("剪貼區 簡體字->正體字")));
  gtk_box_pack_start (GTK_BOX (vbox), button_gb_translate_toggle, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_gb_translate_toggle), "clicked",
                    G_CALLBACK (cb_gb_translate_toggle), NULL);

  GtkWidget *button_juying_learn_toggle = gtk_button_new_with_label(_(_L("剪貼區 注音查詢")));
  gtk_box_pack_start (GTK_BOX (vbox), button_juying_learn_toggle, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_juying_learn_toggle), "clicked",
                    G_CALLBACK (cb_juying_learn), NULL);

  GtkWidget *expander_ts = gtk_expander_new (_(_L("詞庫選項")));
  gtk_box_pack_start (GTK_BOX (vbox), expander_ts, FALSE, FALSE, 0);
  g_signal_connect (expander_ts, "notify::expanded",
                  G_CALLBACK (expander_callback), NULL);

  GtkWidget *vbox_ts = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_ts), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (expander_ts), vbox_ts);


  GtkWidget *button_ts_export = gtk_button_new_with_label(_(_L("詞庫匯出")));
  gtk_widget_set_hexpand (button_ts_export, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_export, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_ts_export), "clicked",
                    G_CALLBACK (cb_ts_export), NULL);

  GtkWidget *button_ts_import = gtk_button_new_with_label(_(_L("詞庫匯入")));
  gtk_widget_set_hexpand (button_ts_import, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_import, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_ts_import), "clicked",
                    G_CALLBACK (cb_ts_import), NULL);

  GtkWidget *button_ts_edit = gtk_button_new_with_label(_(_L("詞庫編輯")));
  gtk_widget_set_hexpand (button_ts_edit, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_edit, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_edit), "clicked",
                    G_CALLBACK (cb_ts_edit), NULL);

  if (inmd[default_input_method].method_type == method_type_TSIN) {
  GtkWidget *button_hime_tslearn = gtk_button_new_with_label(_(_L("讓詞音從文章學習詞")));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_hime_tslearn, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_hime_tslearn), "clicked",
                    G_CALLBACK (cb_hime_tslearn), NULL);

  GtkWidget *button_ts_import_sys = gtk_button_new_with_label(_(_L("匯入系統的詞庫")));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_import_sys, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_import_sys), "clicked",
                    G_CALLBACK (cb_ts_import_sys), NULL);
  }

  GtkWidget *button_about = gtk_button_new_with_label(_(_L("關於 hime")));
  gtk_box_pack_start (GTK_BOX (vbox), button_about, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_about), "clicked",
                    G_CALLBACK (create_about_window),  NULL);


  GtkWidget *button_help = gtk_button_new_from_stock (GTK_STOCK_HELP);
  gtk_box_pack_start (GTK_BOX (vbox), button_help, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_help), "clicked",
                    G_CALLBACK (cb_help), NULL);

#if 0
  char *pid = getenv("HIME_PID");
  if (pid && (hime_pid = atoi(pid))) {
    GtkWidget *button_hime_exit = gtk_button_new_with_label (_(_L("結束 hime")));
    gtk_box_pack_start (GTK_BOX (vbox), button_hime_exit, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (button_hime_exit), "clicked",
                      G_CALLBACK (cb_hime_exit), NULL);
  }
#endif


  GtkWidget *button_quit = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  gtk_box_pack_start (GTK_BOX (vbox), button_quit, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_quit), "clicked",
                    G_CALLBACK (close_application), NULL);

  gtk_widget_show_all(main_window);
}


void init_TableDir(), exec_setup_scripts();
#if WIN32
void init_hime_program_files();
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif


int main(int argc, char **argv)
{
//  char *messages=getenv("LC_MESSAGES");
#if 0
  char *ctype=getenv("LC_CTYPE");
  if (!(ctype && strstr(ctype, "zh_CN")))
    putenv("LANGUAGE=zh_TW.UTF-8");
#endif

  set_is_chs();


#if UNIX
  setenv("HIME_BIN_DIR", HIME_BIN_DIR, TRUE);
  setenv("UTF8_EDIT", utf8_edit, TRUE);
#endif

  exec_setup_scripts();

  init_TableDir();

  load_setttings();

  load_gtab_list(FALSE);

  gtk_init(&argc, &argv);

#if HIME_i18n_message
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif

  g_object_get(gtk_settings_get_default(), "gtk-alternative-button-order", &button_order, NULL);

  create_main_win();

  // once you invoke hime-setup, the left-right buton tips is disabled
  save_hime_conf_int(LEFT_RIGHT_BUTTON_TIPS, 0);

  pclipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);

  gtk_main();

  return 0;
}
