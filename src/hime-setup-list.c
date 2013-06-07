/* Copyright (C) 2005-2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
#include "hime-conf.h"
#include "gtab.h"

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

extern char *default_input_method_str;

/* XXX UI states hold uncommited preference.
 * That's why we need these global variables. */
static GtkWidget *gtablist_widget = NULL;
static GtkWidget *vbox;
static GtkWidget *sw;
static GtkWidget *treeview;
static GtkWidget *check_button_phonetic_speak, *opt_speaker_opts, *check_button_hime_bell_off;
static GtkWidget *opt_im_toggle_keys, *check_button_hime_remote_client,
       *check_button_hime_shift_space_eng_full,
       *check_button_hime_init_im_enabled,
       *check_button_hime_init_full_mode,
       *check_button_hime_eng_phrase_enabled,
       *check_button_hime_win_sym_click_close,
       *check_button_hime_punc_auto_send;

static GtkWidget *check_button_hime_single_state;
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
  INMD *pinmd;
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
  COLUMN_PINMD,
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
//    dbg("%d] %d\n",i,pinmd->in_cycle);
    foo.default_inmd =  default_input_method == i;
    foo.use = !pinmd->disabled;
    foo.cycle = pinmd->in_cycle && foo.use;
    foo.editable = FALSE;
    foo.pinmd = pinmd;
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
                              G_TYPE_BOOLEAN, G_TYPE_POINTER);

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
                          COLUMN_PINMD,
                          g_array_index (articles, Item, i).pinmd,
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

void save_gtablist_conf ()
{
  if (gtablist_widget == NULL)
  {
    fprintf(stderr, "save_gtablist_conf: gtablist_widget is NULL!\n");
    return;
  }

  GtkTreeModel *model = gtk_tree_view_get_model((GtkTreeView *) treeview);

  GtkTreeIter iter;
  if (!gtk_tree_model_get_iter_first(model, &iter))
    return;

  do {
    char *tkey;
    gtk_tree_model_get(model,&iter, COLUMN_KEY, &tkey, -1);
    gboolean cycle, default_inmd, use;
    gtk_tree_model_get (model, &iter, COLUMN_CYCLE, &cycle, -1);
    gtk_tree_model_get (model, &iter, COLUMN_DEFAULT_INMD, &default_inmd, -1);
    gtk_tree_model_get (model, &iter, COLUMN_USE, &use, -1);
    INMD *pinmd;
    gtk_tree_model_get (model, &iter, COLUMN_PINMD, &pinmd, -1);
    pinmd->in_cycle = cycle;
    pinmd->disabled = !use;
  } while (gtk_tree_model_iter_next(model, &iter));

  dbg("default_input_method_str %s\n",default_input_method_str);
  save_hime_conf_str(DEFAULT_INPUT_METHOD, default_input_method_str);

  int idx;
  idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_im_toggle_keys));
  save_hime_conf_int(HIME_IM_TOGGLE_KEYS, imkeys[idx].keynum);

  free(hime_str_im_cycle);

  int i;
  char tt[512];
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

  save_hime_conf_int(HIME_INIT_FULL_MODE,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_init_full_mode)));

  save_hime_conf_int(HIME_ENG_PHRASE_ENABLED,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_eng_phrase_enabled)));

  save_hime_conf_int(HIME_WIN_SYM_CLICK_CLOSE,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_win_sym_click_close)));

  save_hime_conf_int(HIME_BELL_OFF,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_bell_off)));

  save_hime_conf_int(HIME_PUNC_AUTO_SEND,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_punc_auto_send)));
  save_hime_conf_int(HIME_SINGLE_STATE,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_single_state)));
  if (opt_speaker_opts) {
    idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_speaker_opts));
    save_hime_conf_str(PHONETIC_SPEAK_SEL, pho_speaker[idx]);
  }

  save_gtab_list();

  save_hime_conf_int(PHONETIC_SPEAK,
     gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_phonetic_speak)));

  save_omni_config();
  /* caleb- did not found where "reload" is used.
   * caleb- think the send_hime_message() here does nothing.
   */
  send_hime_message(GDK_DISPLAY(), "reload");
}

void destroy_gtablist_widget()
{
  gtk_widget_destroy(gtablist_widget); gtablist_widget = NULL;
}

int hime_switch_keys_lookup(int key);
#if 1
static gboolean toggled (GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
  GtkTreeModel *model = GTK_TREE_MODEL (data);
  GtkTreeIter iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  gboolean cycle;

  dbg("toggled\n");

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_CYCLE, &cycle, -1);

  cycle ^= 1;
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_CYCLE, cycle, -1);

  if (cycle) {
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
  char *key;
  gtk_tree_model_get (model, &iter, COLUMN_KEY, &key, -1);
  char *file;
  gtk_tree_model_get (model, &iter, COLUMN_FILE, &file, -1);
  char tt[128];
  sprintf(tt, "%s %s", key, file);
  free(default_input_method_str);
  default_input_method_str = strdup(tt);
  dbg("default_input_method_str %s\n", default_input_method_str);
//  default_input_method = hime_switch_keys_lookup(key[0]);

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_DEFAULT_INMD, TRUE, -1);
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
  gboolean cycle, default_inmd, use;
  gtk_tree_model_get (model, &iter, COLUMN_CYCLE, &cycle, -1);
  gtk_tree_model_get (model, &iter, COLUMN_DEFAULT_INMD, &default_inmd, -1);
  gtk_tree_model_get (model, &iter, COLUMN_USE, &use, -1);
  use=!use;
  gboolean must_on = default_inmd;
  dbg("toggle %d %d %d\n", cycle, default_inmd, use);

  if (must_on && !use) {
    return TRUE;
  }

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_USE, use, -1);
  if (!use)
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_CYCLE, FALSE, -1);

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
                                               -1, _("名稱"), renderer,
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
                                               -1, _("Ctrl-Alt-鍵"), renderer,
                                               "text", COLUMN_KEY,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  // cycle column
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),-1,
	  _("在 Ctrl-Shift 中循環"),
	  renderer, "active", COLUMN_CYCLE,
                                               NULL);

  // default_inmd column
  renderer = gtk_cell_renderer_toggle_new ();
  gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled_default_inmd), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _("預設"),

                                               renderer,
                                               "active", COLUMN_DEFAULT_INMD,
                                               NULL);

  // use
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled_use), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _("啟用"),
                                               renderer,
                                               "active", COLUMN_USE,
                                               NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_FILE);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _("檔案名稱"), renderer,
                                               "text", COLUMN_FILE,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);
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

static GtkWidget *create_im_toggle_keys()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);
  GtkWidget *label = gtk_label_new(_("輸入視窗(開啟/關閉)切換"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_im_toggle_keys = gtk_combo_box_new_text ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_im_toggle_keys, FALSE, FALSE, 0);

  int i, current_idx=0;

  for(i=0; imkeys[i].keystr; i++) {
    if (imkeys[i].keynum == hime_im_toggle_keys)
      current_idx = i;
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_im_toggle_keys), imkeys[i].keystr);
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_im_toggle_keys), current_idx);

  return hbox;
}

int get_current_speaker_idx();

static GtkWidget *create_speaker_opts()
{
  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  opt_speaker_opts = gtk_combo_box_new_text ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_speaker_opts, FALSE, FALSE, 0);

  int i;
  int current_idx = get_current_speaker_idx();

  for(i=0; i<pho_speakerN; i++) {
    if (imkeys[i].keynum == hime_im_toggle_keys)
      current_idx = i;
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_speaker_opts), pho_speaker[i]);
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_speaker_opts), current_idx);

  return hbox;
}

#include <dirent.h>

GtkWidget *create_gtablist_widget ()
{
  if (gtablist_widget != NULL)
    fprintf(stderr, "create_gtablist_widget: gtablist_widget was not NULL!\n");

  load_settings();

  GtkWidget *box = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(box), GTK_ORIENTATION_VERTICAL);
  gtablist_widget = box;

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
  gtk_box_pack_start (GTK_BOX (box), vbox, TRUE, TRUE, 0);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_NEVER,
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

  {
    // Trying to get correct size of dialog_data->treeview, then put it into a gtk_scrolled_window
    /* XXX Favonia: is this necessary? */
    gtk_widget_show_all (gtablist_widget);

    GtkRequisition requisition;
    gtk_widget_get_child_requisition (treeview, &requisition);
    gtk_widget_set_size_request(vbox, requisition.width, 240);
  }

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
  gtk_box_pack_start (GTK_BOX (box), vbox, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), create_im_toggle_keys(), FALSE, FALSE, 0);

  GtkWidget *hbox_hime_remote_client = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_remote_client, FALSE, FALSE, 0);
  check_button_hime_remote_client = gtk_check_button_new_with_label (_("支援遠端用戶端程式 (port 9999-)"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_remote_client),check_button_hime_remote_client,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_remote_client),
     hime_remote_client);


  GtkWidget *hbox_hime_init_im_enabled = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_init_im_enabled, FALSE, FALSE, 0);
  check_button_hime_init_im_enabled = gtk_check_button_new_with_label (_("直接進入中文輸入狀態 (限非XIM)"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_init_im_enabled),check_button_hime_init_im_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_init_im_enabled),
     hime_init_im_enabled);

  GtkWidget *hbox_hime_init_full_mode = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_init_full_mode, FALSE, FALSE, 0);
  check_button_hime_init_full_mode = gtk_check_button_new_with_label (_("直接進入全型輸入狀態"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_init_full_mode),check_button_hime_init_full_mode,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_init_full_mode),
     hime_init_full_mode);

  GtkWidget *hbox_hime_shift_space_eng_full = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_shift_space_eng_full, FALSE, FALSE, 0);
  check_button_hime_shift_space_eng_full = gtk_check_button_new_with_label (_("按下 shift-space 進入全形英文模式"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_shift_space_eng_full),check_button_hime_shift_space_eng_full,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_shift_space_eng_full),
     hime_shift_space_eng_full);

  GtkWidget *hbox_hime_single_state = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_single_state, FALSE, FALSE, 0);
  check_button_hime_single_state = gtk_check_button_new_with_label (_("所有程式共用相同的輸入法狀態"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_single_state),check_button_hime_single_state,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_single_state),
     hime_single_state);

  GtkWidget *hbox_hime_eng_phrase_enabled = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_eng_phrase_enabled, FALSE, FALSE, 0);
  check_button_hime_eng_phrase_enabled = gtk_check_button_new_with_label (_("英數狀態下可使用 alt-shift 片語"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_eng_phrase_enabled),check_button_hime_eng_phrase_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_eng_phrase_enabled),
     hime_eng_phrase_enabled);

  GtkWidget *hbox_phonetic_speak = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_phonetic_speak , FALSE, FALSE, 0);
  check_button_phonetic_speak = gtk_check_button_new_with_label (_("輸入的同時念出發音"));
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), check_button_phonetic_speak, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_phonetic_speak), phonetic_speak);


  GtkWidget *hbox_hime_win_sym_click_close = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_win_sym_click_close, FALSE, FALSE, 0);
  check_button_hime_win_sym_click_close = gtk_check_button_new_with_label (_("符號視窗點選後自動關閉"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_win_sym_click_close),check_button_hime_win_sym_click_close,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_win_sym_click_close),
     hime_win_sym_click_close);

  GtkWidget *hbox_hime_bell_off = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_bell_off, FALSE, FALSE, 0);
  check_button_hime_bell_off = gtk_check_button_new_with_label (_("關閉嗶聲"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_bell_off),check_button_hime_bell_off,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_bell_off),
     hime_bell_off);

  GtkWidget *hbox_hime_punc_auto_send = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_hime_punc_auto_send, FALSE, FALSE, 0);
  check_button_hime_punc_auto_send = gtk_check_button_new_with_label (_("結尾標點符號自動送出編輯區內容"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_punc_auto_send),check_button_hime_punc_auto_send,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_punc_auto_send),
     hime_punc_auto_send);

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

  if (pho_speakerN) {
    GtkWidget *labelspeaker = gtk_label_new(_("發音選擇"));
    gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), labelspeaker, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (hbox_phonetic_speak), create_speaker_opts());
  }

  return gtablist_widget;
}
