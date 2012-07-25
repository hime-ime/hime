/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
#include "gtab.h"
#include <signal.h>
#include "gst.h"

#if !GTK_CHECK_VERSION(2,90,6)
extern GdkPixbuf *gdk_pixbuf_get_from_surface(cairo_surface_t *surface, gint src_x, gint src_y, gint width, gint height);
#endif

extern void destroy_other_tray();

GtkStatusIcon *tray_icon;
static GdkPixbuf *pixbuf, *pixbuf_ch;
static PangoLayout* pango;
static cairo_t *cr;
static GtkWidget *tray_menu = NULL;

static int iw, ih;

static GdkColor red_color_fg;
static GdkColor blue_color_fg;

#define HIME_TRAY_PNG "hime-tray.png"
static char pixbuf_ch_fname[512];
void exec_hime_setup();

void toggle_gb_output();
extern gboolean gb_output;

static char full[] = N_("全"), engst[] = N_("ABC"), sim[] = N_("简");
extern int current_shape_mode();

void destroy_tray_icon()
{
  if (tray_icon != NULL) {
// Workaround: to release the space on notification area
  gtk_status_icon_set_visible(tray_icon, FALSE);
  g_object_unref(tray_icon); tray_icon = NULL;
  }
  if (tray_menu) {
    gtk_widget_destroy(tray_menu);
    tray_menu = NULL;
  }
  if (pixbuf) {
    g_object_unref(pixbuf); pixbuf = NULL;
  }
  if (pixbuf_ch) {
    g_object_unref(pixbuf_ch); pixbuf_ch = NULL;
  }
}

static void get_text_w_h(char *s, int *w, int *h)
{
  pango_layout_set_text(pango, s, strlen(s));
  pango_layout_get_pixel_size(pango, w, h);
}

static void draw_icon()
{
  gboolean tsin_pho_mode();

  if (!tray_icon)
    return;

  GdkPixbuf *pix =  ((! current_CS) ||
                     (current_CS->im_state != HIME_STATE_CHINESE)) ?
                    pixbuf : pixbuf_ch;

  int w = 0, h = 0;
  iw = gtk_status_icon_get_size(tray_icon), ih = gtk_status_icon_get_size(tray_icon);

  cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, iw, ih);
  cr = cairo_create (cst);
  gdk_cairo_set_source_color (cr, &red_color_fg);

  if (pix) {
    gdk_cairo_set_source_pixbuf (cr, pix, 0, 0);
    cairo_paint (cr);
  } else {
    get_text_w_h(inmd[current_CS->in_method].cname, &w, &h);
    cairo_move_to (cr, 0, 0);
    pango_cairo_show_layout (cr, pango);
  }

  if (current_CS) {
    gdk_cairo_set_source_color (cr, &red_color_fg);
    if (current_shape_mode()) {
      get_text_w_h(full,  &w, &h);
      cairo_move_to (cr, iw - w, ih - h);
      pango_cairo_show_layout (cr, pango);
    }
    if (current_CS->im_state == HIME_STATE_CHINESE && !tsin_pho_mode()) {
      gdk_cairo_set_source_color (cr, &blue_color_fg);
      get_text_w_h(engst,  &w, &h);
      cairo_move_to (cr, 0, 0);
      pango_cairo_show_layout (cr, pango);
    }
  }

  if (gb_output) {
    gdk_cairo_set_source_color (cr, &red_color_fg);
    get_text_w_h(sim,  &w, &h);
    cairo_move_to (cr, 0, ih - h);
    pango_cairo_show_layout (cr, pango);
  }
  cairo_destroy(cr); cr = NULL;
  GdkPixbuf *icon_pixbuf_output = gdk_pixbuf_get_from_surface(cst, 0, 0, iw, ih);
  cairo_surface_destroy(cst); cst = NULL;
  gtk_status_icon_set_from_pixbuf(tray_icon, icon_pixbuf_output);
  g_object_unref(icon_pixbuf_output); icon_pixbuf_output = NULL;
  pix = NULL;
}

void get_icon_path(char *iconame, char fname[]);
gboolean create_tray(gpointer data);

void load_tray_icon()
{
  if (!hime_status_tray)
    return;
  if (!tray_icon) {
    create_tray(NULL);
    return;
  }
  // wrong width & height if it is not embedded-ready
  if (!gtk_status_icon_is_embedded(tray_icon))
    return;
  iw = gtk_status_icon_get_size(tray_icon), ih = gtk_status_icon_get_size(tray_icon);
  if (!pixbuf) {
    char icon_fname[128];
    get_icon_path(HIME_TRAY_PNG, icon_fname);
    pixbuf = gdk_pixbuf_new_from_file_at_size(icon_fname, iw, ih, NULL);
  }
  char *iconame = HIME_TRAY_PNG;
//  if (current_CS && current_CS->in_method && inmd)
// Workaround due to issue #161
  if (current_CS && current_CS->im_state != HIME_STATE_DISABLED && current_CS->im_state != HIME_STATE_ENG_FULL)
    iconame = inmd[current_CS->in_method].icon;
  char fname[512];
  if (iconame)
    get_icon_path(iconame, fname);
  if (strcmp(pixbuf_ch_fname, fname) && pixbuf_ch) {
    g_object_unref(pixbuf_ch); pixbuf_ch = NULL;
  }
  if (!pixbuf_ch) {
    strcpy(pixbuf_ch_fname, fname);
    pixbuf_ch = gdk_pixbuf_new_from_file_at_size(fname, iw, ih, NULL);
  }
  draw_icon();
  iconame = NULL;
}

void exec_hime_setup_(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_trad_sim_toggle();
void cb_trad_sim_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_sim2trad(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void quit_hime(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_tog_phospeak(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void kbm_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat);
extern gboolean win_kbm_on;

void cb_inmd_menu(GtkCheckMenuItem *checkmenuitem, gpointer dat);

#include "mitem.h"

static MITEM mitems[] = {
  {N_("設定"), GTK_STOCK_PREFERENCES, exec_hime_setup_, NULL},
  {N_("結束hime"), GTK_STOCK_QUIT, quit_hime, NULL},
  {N_("念出發音"), NULL, cb_tog_phospeak, &phonetic_speak},
  {N_("繁轉簡工具"), NULL, cb_trad2sim, NULL},
  {N_("簡轉繁工具"), NULL, cb_sim2trad, NULL},
  {N_("選擇輸入法"), NULL, cb_inmd_menu, NULL},
  {N_("小鍵盤"), NULL, kbm_toggle_, &hime_show_win_kbm},
  {N_("輸出成簡體"), NULL, cb_trad_sim_toggle_, &gb_output},
  {NULL, NULL, NULL, NULL}
};


GtkWidget *create_tray_menu(MITEM *mitems);
void update_item_active_all();

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);

void reload_tray_icon()
{
  if (pixbuf) {
    g_object_unref(pixbuf); pixbuf = NULL;
  }
  if (pixbuf_ch) {
    g_object_unref(pixbuf_ch); pixbuf_ch = NULL;
  }
  load_tray_icon();
}

gboolean tray_size_changed_cb (GtkStatusIcon *status_icon, gint *size, gpointer user_data)
{
  reload_tray_icon();
  return FALSE;
}

gboolean tray_embedded_cb (GtkStatusIcon *status_icon, GParamSpec *pspec, gpointer user_data)
{
  if (gtk_status_icon_is_embedded(tray_icon)) {
    reload_tray_icon();
  }
  return TRUE;
}

void toggle_im_enabled(), kbm_toggle();
gboolean tray_button_press_event_cb (GtkStatusIcon *status_icon, GdkEventButton * event, gpointer userdata)
{
  switch (event->button) {
    case 1:
      if (event->state & GDK_SHIFT_MASK)
        inmd_switch_popup_handler(NULL, (GdkEvent *)event);
      else
        toggle_im_enabled();
      break;
    case 2:
#if 0
      inmd_switch_popup_handler(NULL, (GdkEvent *)event);
#else
      kbm_toggle();
      dbg("win_kbm_on %d\n", win_kbm_on);
      update_item_active_all();
#endif
      break;
    case 3:
      if (!tray_menu)
        tray_menu = create_tray_menu(mitems);
      gtk_menu_popup(GTK_MENU(tray_menu), NULL, NULL, gtk_status_icon_position_menu, tray_icon, event->button, event->time);
      break;
  }

  return TRUE;
}

void update_item_active(MITEM *mitems);

void update_item_active_single()
{
  update_item_active(mitems);
}

gboolean create_tray(gpointer data)
{
  if (tray_icon)
    return FALSE;

  destroy_other_tray();

  tray_icon = gtk_status_icon_new();

  g_signal_connect (G_OBJECT (tray_icon), "button-press-event",
                    G_CALLBACK (tray_button_press_event_cb), NULL);

  g_signal_connect (G_OBJECT (tray_icon), "size-changed",
                    G_CALLBACK (tray_size_changed_cb), NULL);

  g_signal_connect (G_OBJECT (tray_icon), "notify::embedded",
                  G_CALLBACK (tray_embedded_cb), NULL);

#if GTK_CHECK_VERSION(2,12,0)
  gtk_status_icon_set_tooltip_text (tray_icon, _("左:中英切換 中:小鍵盤 右:選項"));
#else
  GtkTooltips *tips = gtk_tooltips_new ();
  gtk_status_icon_set_tooltip (GTK_TOOLTIPS (tips), tray_icon, _("左:中英切換 中:小鍵盤 右:選項"), NULL);
#endif

// Initiate Pango for drawing texts from default setting
  GtkWidget *wi = gtk_label_new (NULL); // for reference
  PangoContext *context = gtk_widget_get_pango_context(wi);
  PangoFontDescription* desc = pango_font_description_copy(pango_context_get_font_description(context)); // Copy one from wi for pango
  pango_font_description_set_size(desc, 9 * PANGO_SCALE);
  pango = gtk_widget_create_pango_layout(wi, NULL);
  pango_layout_set_font_description(pango, desc);
 
  gdk_color_parse("red", &red_color_fg);
  gdk_color_parse("blue", &blue_color_fg);

  gtk_widget_destroy(wi);

  load_tray_icon();
  return FALSE;
}

gboolean is_exist_tray()
{
  return tray_icon != NULL;
}

void init_tray()
{
  g_timeout_add(200, create_tray, NULL); // Old setting is 5000 here.
}
