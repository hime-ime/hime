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
#include "pho.h"
#include "tsin.h"
#include "gst.h"
#include "im-client/hime-im-client-attr.h"
#include "hime-module.h"
#include "hime-module-cb.h"

/* XXX UI states hold uncommited preference.
 * That's why we need these global variables. */
static GtkWidget *check_button_root_style_use,
                 *check_button_hime_pop_up_win,
                 *check_button_hime_inner_frame,
		 *check_button_hime_show_win_kbm,
#if TRAY_ENABLED
                 *check_button_hime_status_tray,
                 *check_button_hime_tray_hf_win_kbm,
#endif
                 *check_button_hime_win_color_use,
                 *check_button_hime_on_the_spot_key;


static GtkWidget *appearance_widget;
static GtkWidget *opt_hime_edit_display;
static GdkColor hime_win_gcolor_fg, hime_win_gcolor_bg, hime_sel_key_gcolor, tsin_cursor_gcolor;
gboolean button_order;
#if TRAY_ENABLED
static GtkWidget *opt_hime_tray_display;
#endif

typedef struct {
  GdkColor *color;
  char **color_str;
  GtkWidget *color_selector;
  unich_t *title;
} COLORSEL;

COLORSEL colorsel[4] =
  { {&hime_win_gcolor_fg, &hime_win_color_fg, NULL, N_("前景顏色")},
    {&hime_win_gcolor_bg, &hime_win_color_bg, NULL, N_("背景顏色")},
    {&hime_sel_key_gcolor, &hime_sel_key_color, NULL, N_("選擇鍵顏色")},
    {&tsin_cursor_gcolor, &tsin_cursor_color, NULL, N_("游標顏色")}
  };

struct {
  unich_t *keystr;
  int keynum;
} edit_disp[] = {
  {N_("輸入視窗"), HIME_EDIT_DISPLAY_OVER_THE_SPOT},
  {N_("應用程式編輯區"), HIME_EDIT_DISPLAY_ON_THE_SPOT},
  {N_("同時顯示"),  HIME_EDIT_DISPLAY_BOTH},
  { NULL, 0},
};

#if TRAY_ENABLED
struct {
  unich_t *keystr;
  int keynum;
} tray_disp[] = {
  {N_("單圖示"), HIME_TRAY_DISPLAY_SINGLE},
  {N_("雙圖示"), HIME_TRAY_DISPLAY_DOUBLE},
#if TRAY_UNITY
  {N_("AppIndicator"),  HIME_TRAY_DISPLAY_APPINDICATOR},
#endif
  { NULL, 0},
};
#endif


static GtkWidget *spinner_hime_font_size_tsin_presel,
                 *spinner_hime_font_size_symbol,*spinner_hime_font_size_pho_near,
                 *spinner_hime_font_size_win_kbm,
                 *spinner_hime_font_size_win_kbm_en,
                 *spinner_hime_font_size_tsin_pho_in, *spinner_hime_font_size_gtab_in, *spinner_root_style_x,
                 *spinner_root_style_y, *font_sel;

static GtkWidget *label_win_color_test, *event_box_win_color_test;

void save_appearance_conf()
{
  if (appearance_widget == NULL)
  {
    fprintf(stderr, "save_appearance_conf: appearance_widget is NULL!\n");
    return;
  }

  char fname[128];
  strcpy(fname, gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_sel)));
  int len = strlen(fname)-1;
  

  while (len > 0 && isdigit(fname[len])) {
       len--;
  }
  save_hime_conf_int(HIME_FONT_SIZE, atoi(&(fname[len+1])));

  while (len > 0 && fname[len]==' ') {
       fname[len--]=0;
  }
  save_hime_conf_str(HIME_FONT_NAME, fname);

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
#endif

  save_hime_conf_int(HIME_WIN_COLOR_USE, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_win_color_use)));
  save_hime_conf_str(HIME_WIN_COLOR_FG, hime_win_color_fg);
  save_hime_conf_str(HIME_WIN_COLOR_BG, hime_win_color_bg);
  save_hime_conf_str(HIME_SEL_KEY_COLOR, hime_sel_key_color);
  save_hime_conf_str(TSIN_CURSOR_COLOR, tsin_cursor_color);

  save_hime_conf_int(HIME_ON_THE_SPOT_KEY, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_on_the_spot_key)));
  save_hime_conf_int(KBM_TOGGLE, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_show_win_kbm)));
#if TRAY_ENABLED
  save_hime_conf_int(HIME_TRAY_HF_WIN_KBM, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_tray_hf_win_kbm)));
#endif

  int idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_hime_edit_display));
  save_hime_conf_int(HIME_EDIT_DISPLAY, edit_disp[idx].keynum);

#if TRAY_ENABLED
  idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_hime_tray_display));
  save_hime_conf_int(HIME_TRAY_DISPLAY, tray_disp[idx].keynum);
#endif

#if 0
  save_hime_conf_int(HIME_SETUP_WINDOW_TYPE_UTILITY, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_hime_setup_window_type_utility)));
#endif

  save_omni_config();
  send_hime_message(GDK_DISPLAY(), CHANGE_FONT_SIZE);
#if TRAY_ENABLED
  send_hime_message(GDK_DISPLAY(), UPDATE_TRAY);
#endif
}

void destroy_appearance_widget ()
{
  gtk_widget_destroy(appearance_widget); appearance_widget = NULL;
}

void disp_win_sample();
static void cb_save_hime_win_color(GtkWidget *widget, gpointer user_data)
{
  COLORSEL *sel = (COLORSEL *)user_data;
  GtkWidget *color_selector = sel->color_selector;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(color_selector))), sel->color);
  *sel->color_str = gtk_color_selection_palette_to_string(sel->color, 1);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_win_color_use), TRUE);
  disp_win_sample();
}

static gboolean cb_hime_win_color( GtkWidget *widget,
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

  if (gtk_dialog_run(GTK_DIALOG(color_selector)) == GTK_RESPONSE_OK)
    cb_save_hime_win_color((GtkWidget *)color_selector, (gpointer) sel);
  gtk_widget_destroy(color_selector);

  return TRUE;
}

void disp_win_sample()
{
  dbg("disp_win_sample\n");
  unich_t tt[512];
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(check_button_hime_win_color_use))) {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, &hime_win_gcolor_bg);
#else
    GdkRGBA rgbbg;
    gdk_rgba_parse(&rgbbg, gdk_color_to_string(&hime_win_gcolor_bg));
    gtk_widget_override_background_color(event_box_win_color_test, GTK_STATE_FLAG_NORMAL, &rgbbg);
#endif

#if PANGO_VERSION_CHECK(1,22,0)
  sprintf
(tt, _("<span foreground=\"%s\" font=\"%d\">7</span><span foreground=\"%s\" font=\"%d\">測</span><span font=\"%d\" foreground=\"white\" background=\"%s\">試</span>"), hime_sel_key_color,
hime_font_size_tsin_presel, hime_win_color_fg, hime_font_size_tsin_presel, hime_font_size_tsin_presel, tsin_cursor_color);
#else
  sprintf
(tt, _("<span foreground=\"%s\" font_desc=\"%d\">7</span><span foreground=\"%s\" font_desc=\"%d\">測</span><span font_desc=\"%d\" foreground=\"white\" background=\"%s\">試</span>"), hime_sel_key_color,
hime_font_size_tsin_presel, hime_win_color_fg, hime_font_size_tsin_presel, hime_font_size_tsin_presel, tsin_cursor_color);
#endif
  } else {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, NULL);
#else
    gtk_widget_override_background_color(event_box_win_color_test, GTK_STATE_FLAG_NORMAL, NULL);
#endif

#if PANGO_VERSION_CHECK(1,22,0)
  sprintf
(tt, _("<span foreground=\"blue\" font=\"%d\">7</span><span font=\"%d\">測</span><span font=\"%d\" foreground=\"white\" background=\"blue\">試</span>"), hime_font_size_tsin_presel, hime_font_size_tsin_presel, hime_font_size_tsin_presel);
#else
  sprintf
(tt, _("<span foreground=\"blue\" font_desc=\"%d\">7</span><span font_desc=\"%d\">測</span><span font_desc=\"%d\" foreground=\"white\" background=\"blue\">試</span>"), hime_font_size_tsin_presel, hime_font_size_tsin_presel, hime_font_size_tsin_presel);
#endif

  }

  gtk_label_set_markup(GTK_LABEL(label_win_color_test), _(tt));
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

  GtkWidget *box = gtk_vbox_new (FALSE, 1);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(box), GTK_ORIENTATION_VERTICAL);

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);
  GtkWidget *label = gtk_label_new(_("編輯區顯示"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_hime_edit_display = gtk_combo_box_new_text ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_hime_edit_display, FALSE, FALSE, 0);

  int i, current_idx=0;

  for(i=0; edit_disp[i].keystr; i++) {
    if (edit_disp[i].keynum == hime_edit_display)
      current_idx = i;
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_hime_edit_display), _(edit_disp[i].keystr));
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_hime_edit_display), current_idx);

  check_button_hime_on_the_spot_key = gtk_check_button_new_with_label (_("顯示字根於應用程式中 (OnTheSpot)"));
  g_signal_connect (G_OBJECT (check_button_hime_on_the_spot_key), "toggled",
                    G_CALLBACK (cb_button_hime_on_the_spot_key), NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_on_the_spot_key),
       hime_on_the_spot_key);
  gtk_box_pack_start (GTK_BOX (box), check_button_hime_on_the_spot_key, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(opt_hime_edit_display), "changed",
        G_CALLBACK(combo_selected), (gpointer) NULL);

  return box;
}

#if TRAY_ENABLED
static GtkWidget *create_hime_tray_display()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  check_button_hime_status_tray = gtk_check_button_new_with_label (_("啟用 System Tray Icon"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_status_tray),
       hime_status_tray);
  gtk_box_pack_start (GTK_BOX(hbox), check_button_hime_status_tray, FALSE, FALSE, 0);

//  GtkWidget *label = gtk_label_new(_("System tray displays as"));
//  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_hime_tray_display = gtk_combo_box_new_text ();

  gtk_box_pack_start (GTK_BOX (hbox), opt_hime_tray_display, FALSE, FALSE, 0);

  int i, current_idx=0;

  for(i=0; tray_disp[i].keystr; i++) {
    if (tray_disp[i].keynum == hime_tray_display)
      current_idx = i;
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_hime_tray_display), _(tray_disp[i].keystr));
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_hime_tray_display), current_idx);

//  g_signal_connect(G_OBJECT(opt_hime_tray_display), "changed",
//        G_CALLBACK(combo_selected), (gpointer) NULL);

  return hbox;
}
#endif

static gboolean cb_hime_win_color_use(GtkToggleButton *togglebutton, gpointer user_data)
{
  dbg("cb_hime_win_color_use\n");
  disp_win_sample();
  return TRUE;
}

GtkWidget *create_appearance_widget()
{
  if (appearance_widget != NULL)
    fprintf(stderr, "create_appearance_widget: appearance_widget was not NULL!\n");

  load_settings();

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_top), GTK_ORIENTATION_VERTICAL);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_top), 10);
  appearance_widget = vbox_top;

  GtkWidget *hbox_hime_font_size = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size, FALSE, FALSE, 0);

  GtkWidget *label_hime_font_size = gtk_label_new(_("主要字型"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size), label_hime_font_size, FALSE, FALSE, 0);
  char tt[128];
  sprintf(tt, "%s %d", hime_font_name, hime_font_size);
  font_sel = gtk_font_button_new_with_font (tt);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size), font_sel, FALSE, FALSE, 0);

  GtkWidget *hbox_hime_font_size_symbol = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_symbol, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_symbol = gtk_label_new(_("符號選擇視窗字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_symbol), label_hime_font_size_symbol, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_symbol =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_symbol, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_symbol = gtk_spin_button_new (adj_hime_font_size_symbol, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_symbol), spinner_hime_font_size_symbol, FALSE, FALSE, 0);


  GtkWidget *hbox_hime_font_size_tsin_presel = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_tsin_presel, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_tsin_presel = gtk_label_new(_("詞音&gtab預選詞視窗字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_tsin_presel), label_hime_font_size_tsin_presel, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_tsin_presel =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_tsin_presel, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_tsin_presel = gtk_spin_button_new (adj_hime_font_size_tsin_presel, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_tsin_presel), spinner_hime_font_size_tsin_presel, FALSE, FALSE, 0);


  GtkWidget *hbox_hime_font_size_tsin_pho_in = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_tsin_pho_in, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_tsin_pho_in = gtk_label_new(_("詞音注音輸入區字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_tsin_pho_in), label_hime_font_size_tsin_pho_in, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_tsin_pho_in =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_tsin_pho_in, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_tsin_pho_in = gtk_spin_button_new (adj_hime_font_size_tsin_pho_in, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_tsin_pho_in), spinner_hime_font_size_tsin_pho_in, FALSE, FALSE, 0);


  GtkWidget *hbox_hime_font_size_pho_near = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_pho_near, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_pho_near = gtk_label_new(_("詞音近似音顯示字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_pho_near), label_hime_font_size_pho_near, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_pho_near =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_pho_near, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_pho_near = gtk_spin_button_new (adj_hime_font_size_pho_near, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_pho_near), spinner_hime_font_size_pho_near, FALSE, FALSE, 0);


  GtkWidget *hbox_hime_font_size_gtab_in = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_gtab_in, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_gtab_in = gtk_label_new(_("gtab(倉頡…)輸入區字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_gtab_in), label_hime_font_size_gtab_in, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_gtab_in =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_gtab_in, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_gtab_in = gtk_spin_button_new (adj_hime_font_size_gtab_in, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_gtab_in), spinner_hime_font_size_gtab_in, FALSE, FALSE, 0);

  GtkWidget *hbox_hime_font_size_win_kbm = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_hime_font_size_win_kbm, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_win_kbm = gtk_label_new(_("小鍵盤字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_win_kbm), label_hime_font_size_win_kbm, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_win_kbm =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_win_kbm, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_win_kbm = gtk_spin_button_new (adj_hime_font_size_win_kbm, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_win_kbm), spinner_hime_font_size_win_kbm, FALSE, FALSE, 0);
  GtkWidget *label_hime_font_size_win_kbm_en = gtk_label_new(_("英數"));
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_win_kbm), label_hime_font_size_win_kbm_en, FALSE, FALSE, 0);
  GtkAdjustment *adj_hime_font_size_win_kbm_en =
   (GtkAdjustment *) gtk_adjustment_new (hime_font_size_win_kbm_en, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_hime_font_size_win_kbm_en = gtk_spin_button_new (adj_hime_font_size_win_kbm_en, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_hime_font_size_win_kbm), spinner_hime_font_size_win_kbm_en, FALSE, FALSE, 0);

  GtkWidget *hbox_hime_pop_up_win = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_top), hbox_hime_pop_up_win, FALSE, FALSE, 0);
  check_button_hime_pop_up_win = gtk_check_button_new_with_label (_("在有輸入字根時才會彈出輸入視窗"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_pop_up_win),
       hime_pop_up_win);
  gtk_box_pack_start (GTK_BOX(hbox_hime_pop_up_win), check_button_hime_pop_up_win, FALSE, FALSE, 0);

#if TRAY_ENABLED
  gtk_box_pack_start (GTK_BOX(vbox_top), create_hime_tray_display(), FALSE, FALSE, 0);
  check_button_hime_tray_hf_win_kbm = gtk_check_button_new_with_label (_("在全/半形圖示上按左鍵可顯示/關閉螢幕小鍵盤"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_tray_hf_win_kbm),
       hime_tray_hf_win_kbm);
  gtk_box_pack_start (GTK_BOX(vbox_top), check_button_hime_tray_hf_win_kbm, FALSE, FALSE, 0);
#endif
  check_button_hime_show_win_kbm = gtk_check_button_new_with_label (_("啟動時顯示螢幕小鍵盤"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_show_win_kbm), hime_show_win_kbm);
  gtk_box_pack_start (GTK_BOX(vbox_top), check_button_hime_show_win_kbm, FALSE, FALSE, 0);

  GtkWidget *frame_root_style = gtk_frame_new(_("固定輸入視窗位置"));
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_root_style, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_root_style), 3);
  GtkWidget *vbox_root_style = gtk_vbox_new (FALSE, 10);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_root_style), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (frame_root_style), vbox_root_style);

  GtkWidget *hbox_root_style_use = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_root_style), hbox_root_style_use, FALSE, FALSE, 0);
  check_button_root_style_use = gtk_check_button_new_with_label (_("啟用"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_root_style_use),
       hime_input_style == InputStyleRoot);
  gtk_box_pack_start (GTK_BOX(hbox_root_style_use), check_button_root_style_use, FALSE, FALSE, 0);

  GtkWidget *label_fix_location = gtk_label_new(_("固定位置："));
  gtk_box_pack_start (GTK_BOX (hbox_root_style_use), label_fix_location, FALSE, FALSE, 0);

  GtkAdjustment *adj_root_style_x =
   (GtkAdjustment *) gtk_adjustment_new (hime_root_x, 0.0, 5120.0, 1.0, 1.0, 0.0);
  spinner_root_style_x = gtk_spin_button_new (adj_root_style_x, 0, 0);
  gtk_widget_set_hexpand (spinner_root_style_x, TRUE);
  gtk_box_pack_start (GTK_BOX(hbox_root_style_use), spinner_root_style_x, FALSE, FALSE, 0);

  GtkAdjustment *adj_root_style_y =
   (GtkAdjustment *) gtk_adjustment_new (hime_root_y, 0.0, 2880.0, 1.0, 1.0, 0.0);
  spinner_root_style_y = gtk_spin_button_new (adj_root_style_y, 0, 0);
  gtk_widget_set_hexpand (spinner_root_style_y, TRUE);
  gtk_box_pack_start (GTK_BOX(hbox_root_style_use), spinner_root_style_y, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX(vbox_top), create_hime_edit_display(), FALSE, FALSE, 0);

  GtkWidget *hbox_hime_inner_frame = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_top), hbox_hime_inner_frame, FALSE, FALSE, 0);
  check_button_hime_inner_frame = gtk_check_button_new_with_label (_("輸入視窗顯示內框"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_inner_frame),
       hime_inner_frame);
  gtk_box_pack_start (GTK_BOX(hbox_hime_inner_frame), check_button_hime_inner_frame, FALSE, FALSE, 0);

#if 0
  check_button_hime_setup_window_type_utility = gtk_check_button_new_with_label (_("把設定視窗設為 UTILITY"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_hime_setup_window_type_utility),
       hime_setup_window_type_utility);
  gtk_box_pack_start (GTK_BOX(vbox_top), check_button_hime_setup_window_type_utility, FALSE, FALSE, 0);
#endif

  GtkWidget *frame_win_color = gtk_frame_new(_("顏色選擇"));
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_win_color, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_win_color), 1);
  GtkWidget *vbox_win_color = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_win_color), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (frame_win_color), vbox_win_color);

  GtkWidget *hbox_win_color_use = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_win_color), hbox_win_color_use, FALSE, FALSE, 0);
  check_button_hime_win_color_use = gtk_check_button_new_with_label (_("自訂顏色主題"));
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
  GtkWidget *button_fg = gtk_button_new_with_label(_("前景顏色"));
  gtk_widget_set_hexpand (button_fg, TRUE);
  gtk_widget_set_halign (button_fg, GTK_ALIGN_FILL);
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_fg, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_fg), "clicked",
                    G_CALLBACK (cb_hime_win_color), &colorsel[0]);
  gdk_color_parse(hime_win_color_fg, &hime_win_gcolor_fg);
//  gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, &hime_win_gcolor_fg);
  gdk_color_parse(hime_win_color_bg, &hime_win_gcolor_bg);
//  gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, &hime_win_gcolor_bg);

  GtkWidget *button_bg = gtk_button_new_with_label(_("背景顏色"));
  gtk_widget_set_hexpand (button_bg, TRUE);
  gtk_widget_set_halign (button_bg, GTK_ALIGN_FILL);
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_bg, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_bg), "clicked",
                    G_CALLBACK (cb_hime_win_color), &colorsel[1]);

  GtkWidget *button_hime_sel_key_color = gtk_button_new_with_label(_("選擇鍵顏色"));
  gtk_widget_set_hexpand (button_hime_sel_key_color, TRUE);
  gtk_widget_set_halign (button_hime_sel_key_color, GTK_ALIGN_FILL);
  g_signal_connect (G_OBJECT (button_hime_sel_key_color), "clicked",
                    G_CALLBACK (cb_hime_win_color), &colorsel[2]);
  gdk_color_parse(hime_sel_key_color, &hime_sel_key_gcolor);
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_hime_sel_key_color, TRUE, TRUE, 0);

  GtkWidget *button_tsin_cursor_color = gtk_button_new_with_label(_("游標顏色"));
  gtk_widget_set_hexpand (button_tsin_cursor_color, TRUE);
  gtk_widget_set_halign (button_tsin_cursor_color, GTK_ALIGN_FILL);
  g_signal_connect (G_OBJECT (button_tsin_cursor_color), "clicked",
                    G_CALLBACK (cb_hime_win_color), &colorsel[3]);
  gdk_color_parse(tsin_cursor_color, &tsin_cursor_gcolor);
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_tsin_cursor_color, TRUE, TRUE, 0);

  disp_win_sample();

  return appearance_widget;
}

