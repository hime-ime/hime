/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
 * Copyright (C) 2012 tytsim <https://github.com/tytsim>
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

static void create_result_win(const int res, const char *cmd)
{
  GtkWidget *dialog = NULL;
  if (res) {
    dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
      "%s code:%d '%s'\n%s", _("結果失敗"), res, strerror(res), cmd);
  }
  else
  {
    dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
      _("結果成功"));
  }
  if (dialog == NULL) exit(-1);

  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy (dialog);
}

static void cb_ts_export()
{
   GtkWidget *file_selector =
      gtk_file_chooser_dialog_new(
         _("請輸入要匯出的檔案名稱"),
         GTK_WINDOW(main_window),
         GTK_FILE_CHOOSER_ACTION_SAVE,
         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
         GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
         NULL);

   gtk_dialog_set_alternative_button_order(
      GTK_DIALOG(file_selector),
      GTK_RESPONSE_ACCEPT,
      GTK_RESPONSE_CANCEL,
      -1);

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
   char *home = getenv("HOME");
   if (!home)
      home = "";
   if (inmd[default_input_method].method_type==method_type_TSIN) {
     snprintf(cmd, sizeof(cmd),
        "cd %s/.config/hime && "HIME_BIN_DIR"/hime-tsd2a32 %s > tmpfile && cat %s >> tmpfile && "HIME_BIN_DIR"/hime-tsa2d32 tmpfile %s",
        home, tsin32_f, selected_filename, tsin32_f);
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

   GtkWidget *file_selector =
      gtk_file_chooser_dialog_new(
         _("請輸入要匯入的檔案名稱"),
         GTK_WINDOW(main_window),
         GTK_FILE_CHOOSER_ACTION_OPEN,
         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
         NULL);

   gtk_dialog_set_alternative_button_order(
      GTK_DIALOG(file_selector),
      GTK_RESPONSE_ACCEPT,
      GTK_RESPONSE_CANCEL,
      -1);

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

static void cb_tslearn()
{
  system("hime-tslearn &");
  exit(0);
}

static void cb_ts_import_sys()
{
  char tt[512];
  sprintf(tt, "cd ~/.config/hime && "HIME_BIN_DIR"/hime-tsd2a32 %s > tmpfile && "HIME_BIN_DIR"/hime-tsd2a32 %s/%s >> tmpfile && "HIME_BIN_DIR"/hime-tsa2d32 tmpfile",
    tsin32_f, HIME_TABLE_DIR, tsin32_f);
  dbg("exec %s\n", tt);
  system(tt);
}

/* XXX */
void create_about_window();

/* XXX */
#include "pho.h"
#include "tsin.h"
#include "gst.h"
#include "im-client/hime-im-client-attr.h"
#include "hime-module.h"
#include "hime-module-cb.h"
HIME_module_callback_functions *init_HIME_module_callback_functions(char *sofile);

/* The type of the entries in the table.
 * XXX XXX XXX XXX XXX XXX XXX XXX XXX */
typedef struct {
  const char *name;
  GtkWidget *(*create)();
  void (*save)();
  void (*destroy)();
} TAB_ENTRY;

/* XXX hime-setup-gtab.c */
GtkWidget *create_gtab_widget();
void save_gtab_conf();
void destroy_gtab_widget();

/* XXX hime-setup-appearance.c */
GtkWidget *create_appearance_widget();
void save_appearance_conf();
void destroy_appearance_widget();

/* XXX hime-setup-pho.c */
GtkWidget *create_kbm_widget();
void save_kbm_conf();
void destroy_kbm_widget();

/* XXX hime-setup-list.c */
GtkWidget *create_gtablist_widget();
void save_gtablist_conf();
void destroy_gtablist_widget();

GtkWidget *misc_widget = NULL;

static void pack_start_new_button_with_callback(
    GtkBox *box,
    const gchar *label,
    GCallback cb,
    gpointer* user_data)
{
  GtkWidget *button = gtk_button_new_with_label(label);
  if (button == NULL) exit(-1);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 5);
  g_signal_connect (G_OBJECT (button), "clicked", cb, user_data);
}

static GtkWidget *create_misc_widget(void)
{
  GtkWidget *top_widget = gtk_vbox_new (FALSE, 5);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(top_widget), GTK_ORIENTATION_VERTICAL);

  pack_start_new_button_with_callback(GTK_BOX(top_widget),
      _("alt-shift 片語編輯"), G_CALLBACK(cb_alt_shift), NULL);

  pack_start_new_button_with_callback(GTK_BOX(top_widget),
      _("符號表編輯"), G_CALLBACK(cb_symbol_table), NULL);

  if (!hime_status_tray)
  {
    pack_start_new_button_with_callback(GTK_BOX(top_widget),
      _("啟用/關閉簡體字輸出"), G_CALLBACK(cb_gb_output_toggle), NULL);

    pack_start_new_button_with_callback(GTK_BOX(top_widget),
      _("顯示/隱藏輸入法鍵盤"), G_CALLBACK (cb_win_kbm_toggle), NULL);
  }

  pack_start_new_button_with_callback(GTK_BOX(top_widget),
    _("剪貼區 簡體字->正體字"), G_CALLBACK (cb_gb_translate_toggle), NULL);

  pack_start_new_button_with_callback(GTK_BOX(top_widget),
    _("剪貼區 注音查詢"), G_CALLBACK (cb_juying_learn), NULL);

  GtkWidget *frame_ts = gtk_frame_new (_("詞庫選項"));
  gtk_box_pack_start (GTK_BOX (top_widget), frame_ts, FALSE, FALSE, 5);
  GtkWidget *vbox_ts = gtk_vbox_new (FALSE, 5);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_ts), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (frame_ts), vbox_ts);

  pack_start_new_button_with_callback(GTK_BOX(vbox_ts),
    _("詞庫匯出"), G_CALLBACK (cb_ts_export), NULL);

  pack_start_new_button_with_callback(GTK_BOX(vbox_ts),
    _("詞庫匯入"), G_CALLBACK (cb_ts_import), NULL);

  pack_start_new_button_with_callback(GTK_BOX(vbox_ts),
    _("詞庫編輯"), G_CALLBACK (cb_ts_edit), NULL);

  pack_start_new_button_with_callback(GTK_BOX(vbox_ts),
    _("從文章學習詞"), G_CALLBACK (cb_tslearn), NULL);

  pack_start_new_button_with_callback(GTK_BOX(vbox_ts),
    _("匯入系統的詞庫"), G_CALLBACK (cb_ts_import_sys), NULL);

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
    pack_start_new_button_with_callback(GTK_BOX(top_widget),
        tt, G_CALLBACK (f->module_setup_window_create), GINT_TO_POINTER(hime_setup_window_type_utility));
  }

#if 0
  GtkWidget *button_about = gtk_button_new_from_stock (GTK_STOCK_ABOUT);
  gtk_box_pack_start (GTK_BOX (top_widget), button_about, TRUE, TRUE, 5);
  g_signal_connect (G_OBJECT (button_about), "clicked",
                    G_CALLBACK (create_about_window),  NULL);
#endif

  return top_widget;
}

static void dummy () { }

/* The table defining all tabs */
#define TAB_TABLE_SIZE 5
static TAB_ENTRY tab_table[TAB_TABLE_SIZE] =
  { {N_("開啟/關閉/預設輸入法"), create_gtablist_widget, save_gtablist_conf, destroy_gtablist_widget}
  , {N_("外觀設定"), create_appearance_widget, save_appearance_conf, destroy_appearance_widget}
  , {N_("注音/詞音/拼音設定"), create_kbm_widget, save_kbm_conf, destroy_kbm_widget}
  , {N_("倉頡/行列/大易設定"), create_gtab_widget, save_gtab_conf, destroy_gtab_widget}
  , {N_("雜項"), create_misc_widget, dummy, dummy}
  };

static void save_all_tabs(void)
{
  int i = 0; /* non-C99 */
  for (i = 0; i < TAB_TABLE_SIZE; i++)
    tab_table[i].save();
}

static void destroy_all_tabs(void)
{
  int i = 0; /* non-C99 */
  for (i = 0; i < TAB_TABLE_SIZE; i++)
    tab_table[i].destroy();
}

static void dialog_response_handler(GtkDialog *dialog,
                                    gint response,
                                    gpointer user_data)
{
  switch (response)
  {
    case GTK_RESPONSE_OK:
      save_all_tabs();
      break;
    default:
      break;
  }

  /* Clean up */
  destroy_all_tabs();
  gtk_main_quit();
}


static void run_dialog(void)
{
  /* Create the notebook. */
  GtkWidget *notebook = gtk_notebook_new();
  if (notebook == NULL)
  {
    fprintf(stderr, "notebook == NULL?!\n");
    exit(-1);
  }

  /* Put tabs into the notebook. */
  int i = 0; /* non-C99 */
  for (i = 0; i < TAB_TABLE_SIZE; i++)
  {
    GtkWidget *child = tab_table[i].create();
    if (child == NULL)
    {
      fprintf(stderr, "[%d] child == NULL?!\n", i);
      exit(-1);
    }

    GtkWidget *label = gtk_label_new(_(tab_table[i].name));
    if (label == NULL)
    {
      fprintf(stderr, "[%d] label == NULL?! (%s translated to %s)\n", i, tab_table[i].name, _(tab_table[i].name));
      exit(-1);
    }

    if (gtk_notebook_append_page(GTK_NOTEBOOK (notebook), child, label) == -1)
    {
      fprintf(stderr, "[%d] append_page\n", i);
      exit(-1);
    }
  }

  /* Create the dialog */
  GtkWidget *dialog = gtk_dialog_new_with_buttons (
      _("hime 設定/工具"),
      NULL,
      GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OK, GTK_RESPONSE_OK,
      NULL);
  if (dialog == NULL)
  {
    fprintf(stderr, "dialog == NULL?!\n");
    exit(-1);
  }

  /* Alternative button order when button-orde = 1 */
  gtk_dialog_set_alternative_button_order (
      GTK_DIALOG(dialog),
      GTK_RESPONSE_OK,
      GTK_RESPONSE_CANCEL,
      -1);

  /* Put the notebook into the dialog. */
  GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_container_add(GTK_CONTAINER (content_area), notebook);

  /* Run the dialog and save the setting when 'OK' is clicked. */
  g_signal_connect_swapped (
      dialog,
      "response",
      G_CALLBACK (dialog_response_handler),
      NULL);


  gtk_widget_show_all(dialog);

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

#if 0
  // once you invoke hime-setup, the left-right buton tips is disabled
  save_hime_conf_int(LEFT_RIGHT_BUTTON_TIPS, 0);
#endif

  run_dialog();
  gtk_main();
  return 0;
}
