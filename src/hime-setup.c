/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
 * Copyright (C) 2012 tytsim <https://github.com/tytsim>
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
#include "config.h"
#include <signal.h>
#if HIME_i18n_message
#include <libintl.h>
#endif
#include "lang.h"
GtkWidget *main_window;
gboolean button_order;
char utf8_edit[]=HIME_SCRIPT_DIR"/utf8-edit";
static gboolean close_application( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  exit(0);
}
void create_appearance_conf_window();
static void cb_appearance_conf()
{
  create_appearance_conf_window();
}
void create_kbm_window();

static void cb_kbm()
{
  create_kbm_window();
}

static void cb_tslearn()
{
  system("hime-tslearn &");
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
    sprintf(tt, "%s code:%d '%s'\n%s", _("結果失敗"), res, strerror(res), cmd);
  }
  else
    strcpy(tt, _("結果成功"));

  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  if (hime_setup_window_type_utility)
    gtk_window_set_type_hint(GTK_WINDOW(main_window), GDK_WINDOW_TYPE_HINT_UTILITY);
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
       file_selector = gtk_file_chooser_dialog_new(_("請輸入要匯出的檔案名稱"),
                              GTK_WINDOW(main_window),
                              GTK_FILE_CHOOSER_ACTION_SAVE,
                              GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              NULL);
   else
       file_selector = gtk_file_chooser_dialog_new(_("請輸入要匯出的檔案名稱"),
                              GTK_WINDOW(main_window),
                              GTK_FILE_CHOOSER_ACTION_SAVE,
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                              NULL);
   char hime_dir[512];
   get_hime_dir(hime_dir);
   char cmd[512];
   char fname[256];
   char *filename=inmd[default_input_method].filename;
   char tt[256];
   if (inmd[default_input_method].method_type==method_type_TSIN) {
      get_hime_user_fname(tsin32_f, fname);
      gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (file_selector), tsin32_f);
   } else if (filename) {
      strcat(strcpy(tt, filename), ".append.gtab.tsin-db");
      if (!get_hime_user_fname(tt, fname)) {
         strcat(strcpy(tt, filename), ".tsin-db");
         if (!get_hime_user_fname(tt, fname))
            p_err("cannot find %s", fname);
      }
      gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (file_selector), tt);
   }
   
   gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (file_selector), TRUE);
   if (gtk_dialog_run (GTK_DIALOG (file_selector)) == GTK_RESPONSE_ACCEPT) {
       gchar *selected_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_selector));
       snprintf(cmd, sizeof(cmd), HIME_BIN_DIR"/hime-tsd2a32 %s -o %s", fname, selected_filename);
       dbg("exec %s\n", cmd);
       int res = system(cmd);
       res = 0; // some problem in system(), the exit code is not reliable
       create_result_win(res, cmd);
   }
   gtk_widget_destroy (file_selector);
}

static void ts_import(const gchar *selected_filename)
{
   char cmd[256];
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
}

static void cb_ts_import()
{
   /* Create the selector */

   GtkWidget *file_selector;
   if (button_order)
       file_selector = gtk_file_chooser_dialog_new(_("請輸入要匯入的檔案名稱"),
                              GTK_WINDOW(main_window),
                              GTK_FILE_CHOOSER_ACTION_OPEN,
                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              NULL);
   else
       file_selector = gtk_file_chooser_dialog_new(_("請輸入要匯入的檔案名稱"),
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
}

static void cb_ts_edit()
{
  system(HIME_BIN_DIR"/hime-ts-edit");
}


static void cb_ts_import_sys()
{
  char tt[512];
  sprintf(tt, "cd ~/.config/hime && "HIME_BIN_DIR"/hime-tsd2a32 %s > tmpfile && "HIME_BIN_DIR"/hime-tsd2a32 %s/%s >> tmpfile && "HIME_BIN_DIR"/hime-tsa2d32 tmpfile",
    tsin32_f, HIME_TABLE_DIR, tsin32_f);
  dbg("exec %s\n", tt);
  system(tt);
}


static void cb_alt_shift()
{
  char tt[512];
  sprintf(tt, "( cd ~/.config/hime && %s phrase.table ) &", utf8_edit);
  system(tt);
}


static void cb_symbol_table()
{
  char tt[512];
  sprintf(tt, "( cd ~/.config/hime && %s symbol-table ) &", utf8_edit);
  system(tt);
}
void create_gtab_conf_window();

static void cb_gtab_conf()
{
  create_gtab_conf_window();
}


static void cb_gb_output_toggle()
{
  send_hime_message(GDK_DISPLAY(), GB_OUTPUT_TOGGLE);
  exit(0);
}

static void cb_win_kbm_toggle()
{
  send_hime_message(GDK_DISPLAY(), KBM_TOGGLE);
  exit(0);
}

static void cb_gb_translate_toggle()
{
  system(HIME_BIN_DIR"/hime-sim2trad &");
  exit(0);
}


static void cb_juying_learn()
{
  system(HIME_BIN_DIR"/hime-juyin-learn &");
  exit(0);
}

void create_gtablist_window();
static void cb_default_input_method()
{
  create_gtablist_window();
}

void create_about_window();
void set_window_hime_icon(GtkWidget *window);

#include "pho.h"
#include "tsin.h"
#include "gst.h"
#include "im-client/hime-im-client-attr.h"
#include "hime-module.h"
#include "hime-module-cb.h"
HIME_module_callback_functions *init_HIME_module_callback_functions(char *sofile);

static void create_main_win()
{
  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  if (hime_setup_window_type_utility)
    gtk_window_set_type_hint(GTK_WINDOW(main_window), GDK_WINDOW_TYPE_HINT_UTILITY);
  gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

  g_signal_connect (G_OBJECT (main_window), "delete_event",
                     G_CALLBACK (close_application),
                     NULL);

  gtk_window_set_title (GTK_WINDOW (main_window), _("hime 設定/工具"));
  set_window_hime_icon(main_window);
  gtk_window_set_resizable (GTK_WINDOW (main_window), FALSE);

  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (main_window), vbox);

  GtkWidget *button_default_input_method = gtk_button_new_with_label(_("內定輸入法 & 開啟/關閉"));
  gtk_box_pack_start (GTK_BOX (vbox), button_default_input_method, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_default_input_method), "clicked",
                    G_CALLBACK (cb_default_input_method), NULL);

  GtkWidget *button_appearance_conf = gtk_button_new_with_label(_("輸入視窗外觀設定"));
  gtk_box_pack_start (GTK_BOX (vbox), button_appearance_conf, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_appearance_conf), "clicked",
                    G_CALLBACK (cb_appearance_conf), NULL);

  GtkWidget *button_kbm = gtk_button_new_with_label(_("注音/詞音/拼音設定"));
  gtk_box_pack_start (GTK_BOX (vbox), button_kbm, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_kbm), "clicked",
                    G_CALLBACK (cb_kbm), NULL);

  GtkWidget *button_gtab_conf = gtk_button_new_with_label(_("倉頡/行列/大易設定"));
  gtk_box_pack_start (GTK_BOX (vbox), button_gtab_conf, FALSE, FALSE, 0);
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
    strcat(tt, _("設定"));
    GtkWidget *button_module_input_method = gtk_button_new_with_label(tt);
    gtk_box_pack_start (GTK_BOX (vbox), button_module_input_method, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_module_input_method), "clicked",
                    G_CALLBACK (f->module_setup_window_create), GINT_TO_POINTER(hime_setup_window_type_utility));
  }


  GtkWidget *button_alt_shift = gtk_button_new_with_label(_("alt-shift 片語編輯"));
  gtk_box_pack_start (GTK_BOX (vbox), button_alt_shift, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_alt_shift), "clicked",
                    G_CALLBACK (cb_alt_shift), NULL);

  GtkWidget *button_symbol_table = gtk_button_new_with_label(_("符號表編輯"));
  gtk_box_pack_start (GTK_BOX (vbox), button_symbol_table, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_symbol_table), "clicked",
                    G_CALLBACK (cb_symbol_table), NULL);

#if TRAY_ENABLED
  if (!hime_status_tray)
  {
#endif
    GtkWidget *button_gb_output_toggle = gtk_button_new_with_label(_("啟用/關閉簡體字輸出"));
    gtk_box_pack_start (GTK_BOX (vbox), button_gb_output_toggle, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_gb_output_toggle), "clicked",
                      G_CALLBACK (cb_gb_output_toggle), NULL);

    void kbm_open_close(GtkButton *checkmenuitem, gboolean b_show);
    GtkWidget *button_win_kbm_toggle = gtk_button_new_with_label(_("顯示/隱藏輸入法鍵盤"));
    gtk_box_pack_start (GTK_BOX (vbox), button_win_kbm_toggle, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_win_kbm_toggle), "clicked",
                      G_CALLBACK (cb_win_kbm_toggle), NULL);
#if TRAY_ENABLED
  }
#endif
  GtkWidget *button_gb_translate_toggle = gtk_button_new_with_label(_("剪貼區 簡體字->正體字"));
  gtk_box_pack_start (GTK_BOX (vbox), button_gb_translate_toggle, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_gb_translate_toggle), "clicked",
                    G_CALLBACK (cb_gb_translate_toggle), NULL);

  GtkWidget *button_juying_learn_toggle = gtk_button_new_with_label(_("剪貼區 注音查詢"));
  gtk_box_pack_start (GTK_BOX (vbox), button_juying_learn_toggle, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_juying_learn_toggle), "clicked",
                    G_CALLBACK (cb_juying_learn), NULL);

  GtkWidget *expander_ts = gtk_expander_new (_("詞庫選項"));
  gtk_box_pack_start (GTK_BOX (vbox), expander_ts, FALSE, FALSE, 0);

  GtkWidget *vbox_ts = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_ts), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (expander_ts), vbox_ts);


  GtkWidget *button_ts_export = gtk_button_new_with_label(_("詞庫匯出"));
  gtk_widget_set_hexpand (button_ts_export, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_export, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_ts_export), "clicked",
                    G_CALLBACK (cb_ts_export), NULL);

  GtkWidget *button_ts_import = gtk_button_new_with_label(_("詞庫匯入"));
  gtk_widget_set_hexpand (button_ts_import, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_import, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_ts_import), "clicked",
                    G_CALLBACK (cb_ts_import), NULL);

  GtkWidget *button_ts_edit = gtk_button_new_with_label(_("詞庫編輯"));
  gtk_widget_set_hexpand (button_ts_edit, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_edit, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_ts_edit), "clicked",
                    G_CALLBACK (cb_ts_edit), NULL);

  GtkWidget *button_hime_tslearn = gtk_button_new_with_label(_("從文章學習詞"));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_hime_tslearn, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_hime_tslearn), "clicked",
                    G_CALLBACK (cb_tslearn), NULL);

  GtkWidget *button_ts_import_sys = gtk_button_new_with_label(_("匯入系統的詞庫"));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_import_sys, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_ts_import_sys), "clicked",
                    G_CALLBACK (cb_ts_import_sys), NULL);

  GtkWidget *button_about = gtk_button_new_from_stock (GTK_STOCK_ABOUT);
  gtk_box_pack_start (GTK_BOX (vbox), button_about, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_about), "clicked",
                    G_CALLBACK (create_about_window),  NULL);

  GtkWidget *button_quit = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  gtk_box_pack_start (GTK_BOX (vbox), button_quit, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_quit), "clicked",
                    G_CALLBACK (close_application), NULL);

  gtk_widget_show_all(main_window);
}


void init_TableDir(), exec_setup_scripts();

int main(int argc, char **argv)
{
  set_is_chs();

  exec_setup_scripts();

  init_TableDir();

  load_settings();

  load_gtab_list(FALSE);

  gtk_init(&argc, &argv);

#if HIME_i18n_message
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif

  g_object_get(gtk_settings_get_default(), "gtk-alternative-button-order", &button_order, NULL);

  create_main_win();

#if 0
  // once you invoke hime-setup, the left-right buton tips is disabled
  save_hime_conf_int(LEFT_RIGHT_BUTTON_TIPS, 0);
#endif

  gtk_main();

  return 0;
}
