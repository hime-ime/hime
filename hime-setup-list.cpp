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
#include "hime-conf.h"
#include "gtab.h"
#include "gtab-list.h"

struct {
  char *keystr;
  int keynum;
} imkeys[] = {
  {"Control-Space", Control_Space},
  {"Shift-Space", Shift_Space},
  {"Alt-Space", Alt_Space},
  {"Windows-Space", Windows_Space},
  { NULL, 0},
};

#if 0
unich_t *gcb_pos[] = {
N_(_L("左下")), N_(_L("左上")), N_(_L("右下")), N_(_L("右上"))
};
#endif

static GtkWidget *gtablist_window = NULL;
static GtkWidget *vbox;
static GtkWidget *hbox;
static GtkWidget *sw;
static GtkWidget *treeview;
static GtkWidget *button, *button2, *check_button_phonetic_speak, *opt_speaker_opts, *check_button_hime_bell_off;
static GtkWidget *opt_im_toggle_keys, *check_button_hime_remote_client,
#if 0
       *check_button_gcb_enabled,
       *opt_gcb_pos,
#endif
       *check_button_hime_shift_space_eng_full,
       *check_button_hime_init_im_enabled,
       *check_button_hime_eng_phrase_enabled,
       *check_button_hime_win_sym_click_close;
#if 0
static GtkWidget *spinner_gcb_position_x, *spinner_gcb_position_y;
#endif
#if UNIX
static GtkWidget *check_button_hime_single_state;
#endif
extern gboolean button_order;

char *pho_speaker[16];
int pho_speakerN;

typedef struct
{
  gchar *name;
  GdkPixbuf *icon;
  gchar *key;
  gchar *file;
  gboolean cycle;
  gboolean default_inmd;
  gboolean use;
  gboolean editable;
} Item;

enum
{
  COLUMN_NAME,
  COLUMN_ICON,
  COLUMN_KEY,
  COLUMN_FILE,
  COLUMN_CYCLE,
  COLUMN_DEFAULT_INMD,
  COLUMN_USE,
  COLUMN_EDITABLE,
  NUM_COLUMNS
};


static GArray *articles = NULL;
int hime_switch_keys_lookup(int key);

#if 0
/* unused */
static int qcmp_key(const void *aa, const void *bb)
{
  Item *a=(Item *)aa, *b=(Item *)bb;

  return hime_switch_keys_lookup(a->key[0]) - hime_switch_keys_lookup(b->key[0]);
}
#endif

extern char *TableDir;
void get_icon_path(char *iconame, char fname[]);

static void
add_items (void)
{
  Item foo;

  g_return_if_fail (articles != NULL);

  load_gtab_list(FALSE);

  int i;
  for (i=0; i < inmdN; i++) {
    INMD *pinmd = &inmd[i];
    char *name = pinmd->cname;
    if (!name)
      continue;

    char key[2];
    char *file = pinmd->filename;
    char *icon = pinmd->icon;

    key[0] = pinmd->key_ch;
    key[1]=0;

    foo.name = g_strdup(name);
    char icon_path[128];
    get_icon_path(icon, icon_path);
    GError *err = NULL;
    foo.icon = gdk_pixbuf_new_from_file(icon_path, &err);
    foo.key = g_strdup(key);
    foo.file = g_strdup(file);
    dbg("%d] %d\n",i,pinmd->in_cycle);
    foo.default_inmd =  default_input_method == i;
    foo.use = !pinmd->disabled;
    foo.cycle = pinmd->in_cycle && foo.use;
    foo.editable = FALSE;
    g_array_append_vals (articles, &foo, 1);
  }

//  g_array_sort (articles,qcmp_key);
}

static GtkTreeModel *
create_model (void)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create array */
  articles = g_array_sized_new (FALSE, FALSE, sizeof (Item), 1);

  add_items ();

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMNS,G_TYPE_STRING, GDK_TYPE_PIXBUF,
                              G_TYPE_STRING,
                              G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
                              G_TYPE_BOOLEAN,
                              G_TYPE_BOOLEAN);

  /* add items */
  for (i = 0; i < articles->len; i++) {
      gtk_list_store_append (model, &iter);

      gtk_list_store_set (model, &iter,
			  COLUMN_NAME,
			  g_array_index (articles, Item, i).name,
			  COLUMN_ICON,
			  g_array_index (articles, Item, i).icon,
			  COLUMN_KEY,
			  g_array_index (articles, Item, i).key,
			  COLUMN_FILE,
			  g_array_index (articles, Item, i).file,
			  COLUMN_CYCLE,
                          g_array_index (articles, Item, i).cycle,
			  COLUMN_DEFAULT_INMD,
                          g_array_index (articles, Item, i).default_inmd,
			  COLUMN_USE,
                          g_array_index (articles, Item, i).use,
                          COLUMN_EDITABLE,
                          g_array_index (articles, Item, i).editable,
                          -1);
  }

  return GTK_TREE_MODEL (model);
}


extern char gtab_list[];

static void save_gtab_list()
{
  char ttt[128];
  get_hime_user_fname(gtab_list, ttt);

  FILE *fp;

  if ((fp=fopen(ttt, "w"))==NULL)
    p_err("cannot write to %s\n", ttt);

  int i;
  for (i=0; i < inmdN; i++) {
    INMD *pinmd = &inmd[i];
    char *name = pinmd->cname;
    if (!name)
      continue;

    char *file = pinmd->filename;
    char *icon = pinmd->icon;
    char *disabled = pinmd->disabled?"!":"";

    fprintf(fp, "%s%s %c %s %s\n", disabled,name, pinmd->key_ch, file, icon);
  }

  fclose(fp);
}


static void cb_ok (GtkWidget *button, gpointer data)
{
  char tt[128];
  tt[0]=inmd[default_input_method].key_ch;
  tt[1]=0;
  save_hime_conf_str(DEFAULT_INPUT_METHOD, tt);

  int idx;
#if UNIX
  idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_im_toggle_keys));
  save_hime_conf_int(HIME_IM_TOGGLE_KEYS, imkeys[idx].keynum);
#else
  save_hime_conf_int(HIME_IM_TOGGLE_KEYS, Control_Space);
#endif

  free(hime_str_im_cycle);

  int i;
  int ttN=0;
  for(i=0;i<inmdN;i++) {
    if (inmd[i].in_cycle) {
      dbg("in %d %c\n", i, inmd[i].key_ch);
      tt[ttN++]=inmd[i].key_ch;
    }
  }
  tt[ttN]=0;
  hime_str_im_cycle = strdup(tt);
  save_hime_conf_str(HIME_STR_IM_CYCLE, hime_str_im_cycle);
  dbg("hime_str_im_cycle ttN:%d '%s' '%s'\n", ttN, hime_str_im_cycle, tt);

  save_hime_conf_int(HIME_REMOTE_CLIENT,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_remote_client)));
  save_hime_conf_int(HIME_SHIFT_SPACE_ENG_FULL,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_shift_space_eng_full)));

  save_hime_conf_int(HIME_INIT_IM_ENABLED,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_init_im_enabled)));

  save_hime_conf_int(HIME_ENG_PHRASE_ENABLED,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_eng_phrase_enabled)));

  save_hime_conf_int(HIME_WIN_SYM_CLICK_CLOSE,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_win_sym_click_close)));

  save_hime_conf_int(HIME_BELL_OFF,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_bell_off)));
#if UNIX
  save_hime_conf_int(HIME_SINGLE_STATE,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_single_state)));
#endif
  if (opt_speaker_opts) {
    idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_speaker_opts));
    save_hime_conf_str(PHONETIC_SPEAK_SEL, pho_speaker[idx]);
  }


#if 0
  save_hime_conf_int(GCB_ENABLED, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcb_enabled)));
  idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_gcb_pos));
  save_hime_conf_int(GCB_POSITION, idx+1); // for backward compatbility
  int pos_x = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcb_position_x));
  save_hime_conf_int(GCB_POSITION_X, pos_x);
  int pos_y = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcb_position_y));
  save_hime_conf_int(GCB_POSITION_Y, pos_y);
#endif

  save_gtab_list();

  save_hime_conf_int(PHONETIC_SPEAK,
     gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_phonetic_speak)));

  gtk_widget_destroy(gtablist_window); gtablist_window = NULL;

  send_hime_message(
#if UNIX
	  GDK_DISPLAY(),
#endif
	  "reload");
}

static void cb_cancel (GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy(gtablist_window); gtablist_window = NULL;
}

int hime_switch_keys_lookup(int key);
#if 1
static gboolean toggled (GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
  GtkTreeModel *model = GTK_TREE_MODEL (data);
  GtkTreeIter iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  gboolean value;

  dbg("toggled\n");

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_CYCLE, &value, -1);
  int i = gtk_tree_path_get_indices (path)[0];

  dbg("i %d\n", i);

//  char *key=g_array_index (articles, Item, i).key;
  int in_no = i;

  if (in_no < 0)
    return TRUE;

  value ^= 1;
  inmd[in_no].in_cycle = value;

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_CYCLE, value, -1);

  if (value) {
    inmd[in_no].disabled = 0;
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_USE, TRUE, -1);
  }

  gtk_tree_path_free (path);

  return TRUE;
}
#endif

static void clear_col_default_inmd(GtkTreeModel *model)
{
  GtkTreeIter iter;

  if (!gtk_tree_model_get_iter_first(model, &iter))
    return;

  do {
    char *tkey;
    gtk_tree_model_get(model,&iter, COLUMN_KEY, &tkey, -1);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_DEFAULT_INMD, 0, -1);

  } while (gtk_tree_model_iter_next(model, &iter));
}


static gboolean toggled_default_inmd(GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
  GtkTreeModel *model = GTK_TREE_MODEL (data);
  clear_col_default_inmd(model);
  GtkTreeIter iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);

//  dbg("toggled_default_inmd\n");
  gtk_tree_model_get_iter (model, &iter, path);
  int i = gtk_tree_path_get_indices (path)[0];
  char *key=g_array_index (articles, Item, i).key;
  default_input_method = hime_switch_keys_lookup(key[0]);
  dbg("default_input_method %d %c\n", default_input_method, key[0]);

  if (default_input_method < 0)
    default_input_method = hime_switch_keys_lookup('6');

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_DEFAULT_INMD, TRUE, -1);

  inmd[default_input_method].disabled = 0;
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_USE, TRUE, -1);

  gtk_tree_path_free (path);

  return TRUE;
}

static gboolean toggled_use(GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
  GtkTreeModel *model = GTK_TREE_MODEL (data);
//  clear_all(model);
  GtkTreeIter iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);

//  dbg("toggled_default_inmd\n");
  gtk_tree_model_get_iter (model, &iter, path);
  int i = gtk_tree_path_get_indices (path)[0];
  char *key=g_array_index (articles, Item, i).key;
  int input_method = i;
  dbg("toggle use %d %c\n", input_method, key[0]);
  gboolean must_on = inmd[input_method].in_cycle || default_input_method==input_method;

  if (must_on && !inmd[input_method].disabled) {
//    dbg("must_on\n");
    return TRUE;
  }

  inmd[input_method].disabled ^= 1;
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_USE, !inmd[input_method].disabled, -1);
  gtk_tree_path_free (path);

  return TRUE;
}


static void
add_columns (GtkTreeView *treeview)
{
  GtkCellRenderer *renderer;
  GtkTreeModel *model = gtk_tree_view_get_model (treeview);

  /* name column */

  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_NAME);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _(_L("名稱")), renderer,
                                               "text", COLUMN_NAME,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  renderer = gtk_cell_renderer_pixbuf_new();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_ICON);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "圖示", renderer,
                                               "pixbuf", COLUMN_ICON,
//                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_KEY);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _(_L("Ctrl-Alt-鍵")), renderer,
                                               "text", COLUMN_KEY,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_FILE);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _(_L("檔案名稱")), renderer,
                                               "text", COLUMN_FILE,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  // cycle column
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),-1,
#if UNIX
	  _(_L("在 Ctrl-Shift 中循環")),
#else
	  _(_L("Ctrl-Shift 循環(必須關閉Windows按鍵")),
#endif
	  renderer, "active", COLUMN_CYCLE,
                                               NULL);

  // default_inmd column
  renderer = gtk_cell_renderer_toggle_new ();
  gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled_default_inmd), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _(_L("預設")),

                                               renderer,
                                               "active", COLUMN_DEFAULT_INMD,
                                               NULL);

  // use
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled_use), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _(_L("啟用")),
                                               renderer,
                                               "active", COLUMN_USE,
                                               NULL);
}


static void callback_win_delete()
{
  gtk_widget_destroy(gtablist_window); gtablist_window = NULL;
}

void set_selection_by_key(int key)
{
  if (!treeview)
    return;

  GtkTreeSelection *selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  GtkTreeIter iter;
  gboolean found=FALSE;

  if (!gtk_tree_model_get_iter_first(model, &iter))
    return;

  do {
    char *tkey;

    gtk_tree_model_get(model,&iter, COLUMN_KEY, &tkey,-1);

    if (atoi(tkey) == key) {
      found=TRUE;
      break;
    }
  } while (gtk_tree_model_iter_next(model, &iter));

  if (found)
    gtk_tree_selection_select_iter(selection,&iter);
}

#if UNIX
static GtkWidget *create_im_toggle_keys()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);
  GtkWidget *label = gtk_label_new(_(_L("輸入視窗(開啟/關閉)切換")));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_im_toggle_keys = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
  GtkWidget *menu_im_toggle_keys = gtk_menu_new ();
#endif
  gtk_box_pack_start (GTK_BOX (hbox), opt_im_toggle_keys, FALSE, FALSE, 0);

  int i, current_idx=0;

  for(i=0; imkeys[i].keystr; i++) {
#if !GTK_CHECK_VERSION(2,4,0)
    GtkWidget *item = gtk_menu_item_new_with_label (imkeys[i].keystr);
#endif

    if (imkeys[i].keynum == hime_im_toggle_keys)
      current_idx = i;

#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_im_toggle_keys), imkeys[i].keystr);
#else
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_im_toggle_keys), item);
#endif
  }

#if !GTK_CHECK_VERSION(2,4,0)
  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_im_toggle_keys), menu_im_toggle_keys);
#endif
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_im_toggle_keys), current_idx);

  return hbox;
}
#endif

int get_current_speaker_idx();

static GtkWidget *create_speaker_opts()
{
  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  opt_speaker_opts = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
  GtkWidget *menu_speaker_opts = gtk_menu_new ();
#endif
  gtk_box_pack_start (GTK_BOX (hbox), opt_speaker_opts, FALSE, FALSE, 0);

  int i;
  int current_idx = get_current_speaker_idx();

  for(i=0; i<pho_speakerN; i++) {
#if !GTK_CHECK_VERSION(2,4,0)
    GtkWidget *item = gtk_menu_item_new_with_label (pho_speaker[i]);
#endif

    if (imkeys[i].keynum == hime_im_toggle_keys)
      current_idx = i;

#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_speaker_opts), pho_speaker[i]);
#else
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_speaker_opts), item);
#endif
  }

#if !GTK_CHECK_VERSION(2,4,0)
  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_speaker_opts), menu_speaker_opts);
#endif
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_speaker_opts), current_idx);

  return hbox;
}


#if 0
static GtkWidget *create_gcb_pos_opts()
{
  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  opt_gcb_pos = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
  GtkWidget *menu_gcb_pos = gtk_menu_new ();
#endif
  gtk_box_pack_start (GTK_BOX (hbox), opt_gcb_pos, FALSE, FALSE, 0);

  int i;

  for(i=0; i<sizeof(gcb_pos)/sizeof(gcb_pos[0]); i++) {
#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_gcb_pos), _(gcb_pos[i]));
#else
    GtkWidget *item = gtk_menu_item_new_with_label (_(gcb_pos[i]));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_gcb_pos), item);
#endif
  }

#if !GTK_CHECK_VERSION(2,4,0)
  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_gcb_pos), menu_gcb_pos);
#endif
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_gcb_pos), gcb_position-1); // for backward compatibily

  return hbox;
}
#endif

#if UNIX
#include <dirent.h>
#endif

void create_gtablist_window (void)
{
  if (gtablist_window) {
    gtk_window_present(GTK_WINDOW(gtablist_window));
    return;
  }

  /* create gtab_list_window, etc */
  gtablist_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(gtablist_window), GTK_WIN_POS_MOUSE);

  gtk_window_set_has_resize_grip(GTK_WINDOW(gtablist_window), FALSE);
 gtk_window_set_title (GTK_WINDOW (gtablist_window), _(_L("輸入法選擇")));
  gtk_container_set_border_width (GTK_CONTAINER (gtablist_window), 1);

  g_signal_connect (G_OBJECT (gtablist_window), "destroy",
                    G_CALLBACK (gtk_widget_destroyed), &gtablist_window);

  g_signal_connect (G_OBJECT (gtablist_window), "delete_event",
                      G_CALLBACK (callback_win_delete), NULL);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (gtablist_window), vbox);

  gtk_box_pack_start (GTK_BOX (vbox),
                      gtk_label_new (_(_L("hime 輸入法選擇"))),
                      FALSE, FALSE, 0);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

  /* create model */
  GtkTreeModel *model = create_model ();

  /* create tree view */
  treeview = gtk_tree_view_new_with_model (model);
  gtk_widget_set_hexpand (treeview, TRUE);
  gtk_widget_set_vexpand (treeview, TRUE);
  g_object_unref (G_OBJECT (model));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);

  GtkTreeSelection *tree_selection =
     gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  gtk_tree_selection_set_mode (tree_selection, GTK_SELECTION_SINGLE);

  add_columns (GTK_TREE_VIEW (treeview));

  gtk_container_add (GTK_CONTAINER (sw), treeview);

#if UNIX
  gtk_box_pack_start (GTK_BOX (vbox), create_im_toggle_keys(), FALSE, FALSE, 0);
#endif

#if UNIX
  GtkWidget *hbox_hime_remote_client = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_remote_client, FALSE, FALSE, 0);
  check_button_hime_remote_client = gtk_check_button_new_with_label (_(_L("支援遠端用戶端程式 (port 9999-)")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_remote_client),check_button_hime_remote_client,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_remote_client),
     hime_remote_client);


  GtkWidget *hbox_hime_init_im_enabled = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_init_im_enabled, FALSE, FALSE, 0);
  check_button_hime_init_im_enabled = gtk_check_button_new_with_label (_(_L("直接進入中文輸入狀態 (限非XIM)")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_init_im_enabled),check_button_hime_init_im_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_init_im_enabled),
     hime_init_im_enabled);
#endif


  GtkWidget *hbox_hime_shift_space_eng_full = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_shift_space_eng_full, FALSE, FALSE, 0);
  check_button_hime_shift_space_eng_full = gtk_check_button_new_with_label (_(_L("按下 shift-space 進入全形英文模式")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_shift_space_eng_full),check_button_hime_shift_space_eng_full,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_shift_space_eng_full),
     hime_shift_space_eng_full);

#if UNIX
  GtkWidget *hbox_hime_single_state = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_single_state, FALSE, FALSE, 0);
  check_button_hime_single_state = gtk_check_button_new_with_label (_(_L("所有程式共用相同的輸入法狀態")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_single_state),check_button_hime_single_state,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_single_state),
     hime_single_state);
#endif

  GtkWidget *hbox_hime_eng_phrase_enabled = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_eng_phrase_enabled, FALSE, FALSE, 0);
  check_button_hime_eng_phrase_enabled = gtk_check_button_new_with_label (_(_L("英數狀態下可使用 alt-shift 片語")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_eng_phrase_enabled),check_button_hime_eng_phrase_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_eng_phrase_enabled),
     hime_eng_phrase_enabled);

  GtkWidget *hbox_phonetic_speak = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_phonetic_speak , FALSE, FALSE, 0);
  check_button_phonetic_speak = gtk_check_button_new_with_label (_(_L("輸入的同時念出發音")));
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), check_button_phonetic_speak, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_phonetic_speak), phonetic_speak);


  GtkWidget *hbox_hime_win_sym_click_close = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_win_sym_click_close, FALSE, FALSE, 0);
  check_button_hime_win_sym_click_close = gtk_check_button_new_with_label (_(_L("符號視窗點選後自動關閉")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_win_sym_click_close),check_button_hime_win_sym_click_close,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_win_sym_click_close),
     hime_win_sym_click_close);

  check_button_hime_bell_off = gtk_check_button_new_with_label (_(_L("關閉嗶聲")));
  gtk_box_pack_start (GTK_BOX (hbox_hime_win_sym_click_close),check_button_hime_bell_off,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_bell_off),
     hime_bell_off);


#if 0
  GtkWidget *hbox_gcb_pos = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcb_pos, FALSE, FALSE, 0);
  check_button_gcb_enabled = gtk_check_button_new_with_label (_(_L("剪貼區管理視窗位置&開關")));
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), check_button_gcb_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcb_enabled),
     gcb_enabled);

  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), create_gcb_pos_opts(),  FALSE, FALSE, 0);
  GtkAdjustment *adj_gcb_position_x =
   (GtkAdjustment *) gtk_adjustment_new (gcb_position_x, 0.0, 100.0, 1.0, 1.0, 0.0);
  spinner_gcb_position_x = gtk_spin_button_new (adj_gcb_position_x, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), spinner_gcb_position_x, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcb_position_y =
   (GtkAdjustment *) gtk_adjustment_new (gcb_position_y, 0.0, 100.0, 1.0, 1.0, 0.0);
  spinner_gcb_position_y = gtk_spin_button_new (adj_gcb_position_y, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), spinner_gcb_position_y, FALSE, FALSE, 0);
#endif

#if UNIX
  DIR *dir;
  if ((dir=opendir(HIME_OGG_DIR"/ㄧ"))) {
    struct dirent *dire;

    pho_speakerN = 0;
    while ((dire=readdir(dir))) {
      char *name = dire->d_name;

      if (name[0]=='.')
        continue;
      pho_speaker[pho_speakerN++]=strdup(name);
    }
    closedir(dir);

    dbg("pho_speakerN:%d\n", pho_speakerN);

  }
#else
  wchar_t oggdir[256];
  wchar_t hime16[256];
  utf8_to_16(hime_program_files_path, hime16, sizeof(hime16));
  wsprintfW(oggdir, L"%s\\ogg\\ㄧ\\*", hime16);

  WIN32_FIND_DATAW ffd;
  HANDLE hFind = FindFirstFileW(oggdir, &ffd);

  if (INVALID_HANDLE_VALUE != hFind) {
    do {
      char tt[256];
      utf16_to_8(ffd.cFileName, tt, sizeof(tt));
	  if (!strcmp(tt, ".") || !strcmp(tt, ".."))
		  continue;
	  dbg("--- %s\n", tt);
      pho_speaker[pho_speakerN++]=strdup(tt);
    } while (FindNextFileW(hFind, &ffd) != 0);

    FindClose(hFind);
  }
#endif

  if (pho_speakerN) {
    GtkWidget *labelspeaker = gtk_label_new(_(_L("發音選擇")));
    gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), labelspeaker, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (hbox_phonetic_speak), create_speaker_opts());
  }

  hbox = gtk_hbox_new (TRUE, 4);
  gtk_grid_set_column_homogeneous(GTK_GRID(hbox), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_cancel), treeview);
  if (button_order)
    gtk_box_pack_end (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  else
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  button2 = gtk_button_new_from_stock (GTK_STOCK_OK);
  g_signal_connect (G_OBJECT (button2), "clicked",
                    G_CALLBACK (cb_ok), model);
#if !GTK_CHECK_VERSION(2,91,2)
  if (button_order)
    gtk_box_pack_end (GTK_BOX (hbox), button2, TRUE, TRUE, 0);
  else
    gtk_box_pack_start (GTK_BOX (hbox), button2, TRUE, TRUE, 0);
#else
  if (button_order)
    gtk_grid_attach_next_to (GTK_BOX (hbox), button2, button, GTK_POS_LEFT, 1, 1);
  else
    gtk_grid_attach_next_to (GTK_BOX (hbox), button2, button, GTK_POS_RIGHT, 1, 1);
#endif
#if UNIX
  gtk_window_set_default_size (GTK_WINDOW (gtablist_window), 620, 450);
#else
  gtk_window_set_default_size (GTK_WINDOW (gtablist_window), 680, 450);
#endif

  g_signal_connect (G_OBJECT (gtablist_window), "delete_event",
                    G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show_all (gtablist_window);

  set_selection_by_key(default_input_method);

#if 0
  g_signal_connect (G_OBJECT(tree_selection), "changed",
                    G_CALLBACK (callback_row_selected), NULL);
#endif
}
