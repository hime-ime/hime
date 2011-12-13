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
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"
#include "eggtrayicon.h"
#include <signal.h>
#include "gst.h"

#if UNIX
static GdkPixbuf *pixbuf, *pixbuf_ch;
static PangoLayout* pango;
static GtkWidget *da;
#if !GTK_CHECK_VERSION(2,90,6)
static GdkGC *gc;
#else
static cairo_t *gc;
#endif
GdkWindow *tray_da_win;
static EggTrayIcon *egg_tray_icon;

#define HIME_TRAY_PNG "hime-tray.png"
static char *pixbuf_ch_fname;
void exec_hime_setup();

void toggle_gb_output();
extern gboolean gb_output;

static void get_text_w_h(char *s, int *w, int *h)
{
  pango_layout_set_text(pango, s, strlen(s));
  pango_layout_get_pixel_size(pango, w, h);
}


static void draw_icon()
{
  gboolean tsin_pho_mode();
//  dbg("draw_icon\n");
#if 0
  return;
#endif
  if (!da)
    return;

  GdkPixbuf *pix =  !current_CS ||
    (current_CS->im_state == HIME_STATE_DISABLED||current_CS->im_state == HIME_STATE_ENG_FULL) ?
    pixbuf : pixbuf_ch;

#if GTK_CHECK_VERSION(2,17,7)
  GtkAllocation dwdh;
  gtk_widget_get_allocation(da, &dwdh);
  int dw = dwdh.width, dh = dwdh.height;
#else
  int dw = da->allocation.width, dh = da->allocation.height;
#endif
  int w, h;

  GdkColor color_fg;


//  dbg("wh %d,%d\n", dw,dh);

  gdk_color_parse("black", &color_fg);
#if !GTK_CHECK_VERSION(2,90,6)
  gdk_gc_set_rgb_fg_color(gc, &color_fg);
#else
  gc = gdk_cairo_create (tray_da_win);
  gdk_cairo_set_source_color (gc, &color_fg);
#endif

  if (pix) {
    int ofs = (dh - gdk_pixbuf_get_height (pix))/2;
#if !GTK_CHECK_VERSION(2,90,6)
    gdk_draw_pixbuf(tray_da_win, NULL, pix, 0, 0, 0, ofs, -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
#else
    gdk_cairo_set_source_pixbuf (gc, pix, 0, ofs);
    cairo_paint (gc);
    cairo_destroy (gc);
#endif
  } else {
    get_text_w_h(inmd[current_CS->in_method].cname, &w, &h);
#if !GTK_CHECK_VERSION(2,90,6)
    gdk_draw_layout(tray_da_win, gc, 0, 0, pango);
#else
    cairo_move_to (gc, 0, 0);
    pango_cairo_show_layout (gc, pango);
    cairo_destroy (gc);
#endif
  }

  if (current_CS) {
    if (current_CS->b_half_full_char ||
#if USE_TSIN
        current_method_type()==method_type_TSIN && tss.tsin_half_full &&
#endif
        current_CS->im_state == HIME_STATE_CHINESE) {
      static char full[] = "全";
      get_text_w_h(full,  &w, &h);
#if !GTK_CHECK_VERSION(2,90,6)
      gdk_draw_layout(tray_da_win, gc, dw - w, dh - h, pango);
#else
      cairo_move_to (gc, dw - w, dh - h);
      pango_cairo_show_layout (gc, pango);
      cairo_destroy (gc);
#endif
    }

    if (current_CS->im_state == HIME_STATE_ENG_FULL) {
      static char efull[] = "A全";
      get_text_w_h(efull,  &w, &h);
#if !GTK_CHECK_VERSION(2,90,6)
      gdk_draw_layout(tray_da_win, gc, 0, 0, pango);
#else
      cairo_move_to (gc, 0, 0);
      pango_cairo_show_layout (gc, pango);
      cairo_destroy (gc);
#endif
    }
#if USE_TSIN
    if ((current_method_type()==method_type_TSIN||current_method_type()==method_type_MODULE) && current_CS->im_state == HIME_STATE_CHINESE && !tsin_pho_mode()) {
      static char efull[] = "ABC";
      gdk_color_parse("blue", &color_fg);
#if !GTK_CHECK_VERSION(2,90,6)
      gdk_gc_set_rgb_fg_color(gc, &color_fg);
#else
      gc = gdk_cairo_create (tray_da_win);
      gdk_cairo_set_source_color (gc, &color_fg);
#endif

      get_text_w_h(efull,  &w, &h);
#if !GTK_CHECK_VERSION(2,90,6)
      gdk_draw_layout(tray_da_win, gc, 0, 0, pango);
#else
      cairo_move_to (gc, 0, 0);
      pango_cairo_show_layout (gc, pango);
      cairo_destroy (gc);
#endif
    }
#endif
  }

  gdk_color_parse("red", &color_fg);
#if !GTK_CHECK_VERSION(2,90,6)
  gdk_gc_set_rgb_fg_color(gc, &color_fg);
#else
  gc = gdk_cairo_create (tray_da_win);
  gdk_cairo_set_source_color (gc, &color_fg);
#endif

  if (gb_output) {
    static char sim[] = "简";
    get_text_w_h(sim,  &w, &h);
#if !GTK_CHECK_VERSION(2,90,6)
    gdk_draw_layout(tray_da_win, gc, 0, dh - h, pango);
#else
    cairo_move_to (gc, 0, dh - h);
    pango_cairo_show_layout (gc, pango);
    cairo_destroy (gc);
#endif
  }

}

gboolean create_tray(gpointer data);
void update_tray_icon()
{
//  dbg("update_tray_icon\n");
  if (!hime_status_tray)
    return;

  if (!da)
    create_tray(NULL);

  gtk_widget_queue_draw(da);
}

void get_icon_path(char *iconame, char fname[]);

void load_tray_icon()
{
//  dbg("load_tray_icon\n");
  if (!hime_status_tray)
    return;

  if (!da)
    create_tray(NULL);

  char *iconame = inmd[current_CS->in_method].icon;
  char fname[512];

  fname[0]=0;

  if (iconame)
    get_icon_path(iconame, fname);

#if GTK_CHECK_VERSION(2,17,7)
  GtkAllocation dwdh;
  gtk_widget_get_allocation(da, &dwdh);
  int dw = dwdh.width, dh = dwdh.height;
#else
    int dw = da->allocation.width, dh = da->allocation.height;
#endif

  if (!pixbuf || gdk_pixbuf_get_width (pixbuf) != dw || gdk_pixbuf_get_height (pixbuf) != dh) {
    char icon_fname[128];
    get_icon_path(HIME_TRAY_PNG, icon_fname);
    GError *err = NULL;
//    dbg("icon_name %s\n", icon_fname);
    pixbuf = gdk_pixbuf_new_from_file_at_size(icon_fname, dw, dh, &err);
    if (!pixbuf)
      p_err("cannot load file %s", icon_fname);
  }

#if 0
  dbg("fname %x %s\n", fname, fname);
#endif
  if (!fname[0]) {
    if (pixbuf_ch)
      g_object_unref(pixbuf_ch);

    pixbuf_ch = NULL;
    if (pixbuf_ch_fname)
      pixbuf_ch_fname[0] = 0;
  } else
  if (!pixbuf_ch_fname || strcmp(fname, pixbuf_ch_fname)) {
    free(pixbuf_ch_fname);
    pixbuf_ch_fname = strdup(fname);

    if (pixbuf_ch)
      g_object_unref(pixbuf_ch);

    dbg("ch %s\n", fname);
    GError *err = NULL;
    pixbuf_ch = gdk_pixbuf_new_from_file_at_size(fname, dw, dh, &err);
  }

  update_tray_icon();
}

void exec_hime_setup_(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_trad_sim_toggle();
void cb_trad_sim_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_hime_sim2trad(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_hime_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void restart_hime(GtkCheckMenuItem *checkmenuitem, gpointer dat);
#endif  // UNIX


void cb_tog_phospeak(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void gcb_main();
void cb_tog_gcb(GtkCheckMenuItem *checkmenuitem, gpointer dat);

#include "mitem.h"

void kbm_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat);
extern int win_kbm_on;

void cb_inmd_menu(GtkCheckMenuItem *checkmenuitem, gpointer dat);

static MITEM mitems[] = {
  {N_("設定"), GTK_STOCK_PREFERENCES, exec_hime_setup_, NULL},
  {N_("重新執行hime"), GTK_STOCK_QUIT, restart_hime, NULL},
  {N_("念出發音"), NULL, cb_tog_phospeak, &phonetic_speak},
#if 0
  {N_("gcb(剪貼區暫存)"), NULL, cb_tog_gcb, &gcb_enabled},
#endif
  {N_("正→簡體"), NULL, cb_hime_trad2sim, NULL},
  {N_("簡→正體"), NULL, cb_hime_sim2trad, NULL},
  {N_("選擇輸入法"), NULL, cb_inmd_menu, NULL},
  {N_("小鍵盤"), NULL, kbm_toggle_, NULL},
  {N_("简体输出"), NULL, cb_trad_sim_toggle_, &gb_output},
  {NULL, NULL, NULL, NULL}
};


static GtkWidget *tray_menu=NULL;

GtkWidget *create_tray_menu(MITEM *mitems);
void update_item_active_all();

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);
extern gboolean win_kbm_inited;


#if UNIX
void toggle_im_enabled(), kbm_toggle();
gboolean
tray_button_press_event_cb (GtkWidget * button, GdkEventButton * event, gpointer userdata)
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

      gtk_menu_popup(GTK_MENU(tray_menu), NULL, NULL, NULL, NULL,
         event->button, event->time);
      break;
  }

  return TRUE;
}

void update_item_active(MITEM *mitems);

void update_item_active_unix()
{
  update_item_active(mitems);
}


#if !GTK_CHECK_VERSION(2,91,0)
gboolean cb_expose(GtkWidget *da, GdkEventExpose *event, gpointer data)
#else
gboolean cb_expose(GtkWidget *da, cairo_t *event, gpointer data)
#endif
{
  if (!da)
    create_tray(NULL);

//  dbg("cb_expose\n");

  draw_icon();
  return FALSE;
}

#if !GTK_CHECK_VERSION(2,90,7)
GdkGC *gdk_gc_new (GdkDrawable *drawable);
#endif

gboolean create_tray(gpointer data)
{
  if (da)
    return FALSE;

  egg_tray_icon = egg_tray_icon_new ("hime");

  if (!egg_tray_icon)
    return FALSE;

  GtkWidget *event_box = gtk_event_box_new ();
// Do not use this, otherwise tray menu fails
//  gtk_event_box_set_visible_window (event_box, FALSE);
  gtk_container_add (GTK_CONTAINER (egg_tray_icon), event_box);
#if GTK_CHECK_VERSION(2,12,0)
  gtk_widget_set_tooltip_text (event_box, _("左:中英切換 中:小鍵盤 右:選項"));
#else
  GtkTooltips *tips = gtk_tooltips_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), event_box, _("左:中英切換 中:小鍵盤 右:選項"), NULL);
#endif

  g_signal_connect (G_OBJECT (event_box), "button-press-event",
                    G_CALLBACK (tray_button_press_event_cb), NULL);

  GError *err = NULL;
  if (pixbuf)
    g_object_unref(pixbuf);

  char icon_fname[128];
  get_icon_path(HIME_TRAY_PNG, icon_fname);
  pixbuf = gdk_pixbuf_new_from_file(icon_fname, &err);
  int pwidth = gdk_pixbuf_get_width (pixbuf);
  int pheight = gdk_pixbuf_get_height (pixbuf);

  da =  gtk_drawing_area_new();
  g_signal_connect (G_OBJECT (event_box), "destroy",
                    G_CALLBACK (gtk_widget_destroyed), &da);
#if !GTK_CHECK_VERSION(2,91,0)
  g_signal_connect(G_OBJECT(da), "expose-event", G_CALLBACK(cb_expose), NULL);
#else
  g_signal_connect(G_OBJECT(da), "draw", G_CALLBACK(cb_expose), NULL);
#endif

  gtk_container_add (GTK_CONTAINER (event_box), da);
  gtk_widget_set_size_request(GTK_WIDGET(egg_tray_icon), pwidth, pheight);

  gtk_widget_show_all (GTK_WIDGET (egg_tray_icon));
  tray_da_win = gtk_widget_get_window(da);
  // tray window is not ready ??
  if (!tray_da_win || !GTK_WIDGET_DRAWABLE(da)) {
    gtk_widget_destroy(GTK_WIDGET(egg_tray_icon));
    da = NULL;
    return FALSE;
  }

  PangoContext *context=gtk_widget_get_pango_context(da);
  PangoFontDescription* desc=pango_context_get_font_description(context);

//  dbg("zz %s %d\n",  pango_font_description_to_string(desc), PANGO_SCALE);

  pango = gtk_widget_create_pango_layout(da, NULL);
  pango_layout_set_font_description(pango, desc);
#if 1
  // strange bug, why do we need this ?
  desc = (PangoFontDescription *)pango_layout_get_font_description(pango);
#endif
  pango_font_description_set_size(desc, 9 * PANGO_SCALE);

#if 0
  dbg("aa %s\n",  pango_font_description_to_string(desc));
  dbg("context %x %x\n", pango_layout_get_context(pango), context);
  dbg("font %x %x\n",pango_layout_get_font_description(pango), desc);
#endif
#if !GTK_CHECK_VERSION(2,90,6)
  gc = gdk_gc_new (tray_da_win);
#endif
  return FALSE;
}

void destroy_tray_icon()
{
  gtk_widget_destroy(GTK_WIDGET(egg_tray_icon));
  egg_tray_icon = NULL; da = NULL;
}

void init_tray()
{
  g_timeout_add(5000, create_tray, NULL);
}
#endif
