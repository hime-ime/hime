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
#include "win-sym.h"

static int current_hime_inner_frame;
static int current_pho_in_row1;

GtkWidget *gwin_pho;
static GtkWidget *top_bin, *hbox_row2;
static GtkWidget *label_pho_sele;
static GtkWidget *label_pho;
static GtkWidget *label_full;
static GtkWidget *label_key_codes;

void change_pho_font_size(), toggle_win_sym();
void disp_pho_sub(GtkWidget *label, int index, char *pho);

void disp_pho(int index, char *phochar)
{
//  dbg("%d '", index); utf8_putchar(phochar); dbg("'\n");
  disp_pho_sub(label_pho, index, phochar);
}

#if WIN32
static int timeout_handle_pho;
gboolean timeout_minimize_win_pho(gpointer data)
{
  if (!gwin_pho)
    return FALSE;
  gtk_window_resize(GTK_WINDOW(gwin_pho), 1, 1);
//  gtk_window_present(GTK_WINDOW(gwin0));
  timeout_handle_pho = 0;
  return FALSE;
}
#endif


void minimize_win_pho()
{
  gtk_window_resize(GTK_WINDOW(gwin_pho), 1, 1);

#if WIN32
  if (!timeout_handle_pho)
	timeout_handle_pho = g_timeout_add(50, timeout_minimize_win_pho, NULL);
#endif
}


void move_win_pho(int x, int y);

void get_win_size(GtkWidget *win, int *width, int *height)
{
  GtkRequisition sz;
  sz.width = sz.height = 0;
  gtk_widget_get_preferred_size(GTK_WIDGET(win), NULL, &sz);
  *width = sz.width;
  *height = sz.height;
}

gboolean win_size_exceed(GtkWidget *win)
{
  int width, height;

  get_win_size(win, &width, &height);

  return (width + current_in_win_x > dpy_xl || height + current_in_win_y > dpy_yl);
}


void disp_pho_sel(char *s)
{
  gtk_label_set_markup(GTK_LABEL(label_pho_sele), s);

  minimize_win_pho();

  if (win_size_exceed(gwin_pho)) {
    move_win_pho(current_in_win_x, current_in_win_y);
  }
}


void set_key_codes_label_pho(char *s)
{
  if (!label_key_codes)
    return;

  if (!s || !*s) {
    gtk_widget_hide(label_key_codes);
	return;
  }

  gtk_label_set_text(GTK_LABEL(label_key_codes), s);
  gtk_widget_show(label_key_codes);
}


void show_win_sym();

void move_win_pho(int x, int y)
{
  int twin_xl, twin_yl;

  win_x = x;  win_y = y;

  if (!gwin_pho)
    return;

  get_win_size(gwin_pho, &twin_xl, &twin_yl);

  if (x + twin_xl > dpy_xl)
    x = dpy_xl - twin_xl;
  if (x < 0)
    x = 0;

  if (y + twin_yl > dpy_yl)
    y = dpy_yl - twin_yl;
  if (y < 0)
    y = 0;

  gtk_window_move(GTK_WINDOW(gwin_pho), x, y);
  move_win_sym();
}


void create_win_pho()
{
  if (gwin_pho)
    return;

  gwin_pho = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(gwin_pho), 1 ,1);
  gtk_window_set_has_resize_grip(GTK_WINDOW(gwin_pho), FALSE);
#if WIN32
  set_no_focus(gwin_pho);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (gwin_pho), 0);
  gtk_widget_realize (gwin_pho);
#if UNIX
  set_no_focus(gwin_pho);
#else
  win32_init_win(gwin_pho);
#endif
  change_win_bg(gwin_pho);
}

void create_win_sym(), exec_hime_setup();

static void mouse_button_callback( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
//  dbg("mouse_button_callback %d\n", event->button);
  switch (event->button) {
    case 1:
      toggle_win_sym();
      break;
    case 2:
      inmd_switch_popup_handler(widget, (GdkEvent *)event);
      break;
    case 3:
      exec_hime_setup();
      break;
  }

}

void create_win_pho_gui_simple()
{
//  dbg("create_win_pho .....\n");

  if (top_bin)
    return;

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_top), GTK_ORIENTATION_VERTICAL);

  GtkWidget *event_box_pho;
  if (gtab_in_area_button)
	event_box_pho = gtk_button_new();
  else {
	event_box_pho = gtk_event_box_new();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX(event_box_pho), FALSE);
  }

  gtk_container_set_border_width (GTK_CONTAINER (event_box_pho), 0);

  if (hime_inner_frame) {
    GtkWidget *frame = top_bin = gtk_frame_new(NULL);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
    gtk_container_add (GTK_CONTAINER(gwin_pho), frame);
    gtk_container_add (GTK_CONTAINER (frame), vbox_top);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  } else {
    gtk_container_add (GTK_CONTAINER(gwin_pho), vbox_top);
    top_bin = vbox_top;
  }


  GtkWidget *align = gtk_alignment_new (0, 0, 0, 0);
  label_pho_sele = gtk_label_new(NULL);

  if (!pho_in_row1) {
    gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (align), label_pho_sele);
  } else {
    GtkWidget *hbox_row1 = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row1, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_row1), event_box_pho, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (hbox_row1), align, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (align), label_pho_sele);
  }


  hbox_row2 = gtk_hbox_new (FALSE, 0);
  /* This packs the button into the gwin_pho (a gtk container). */
  gtk_container_add (GTK_CONTAINER (vbox_top), hbox_row2);
  label_full = gtk_label_new(_(_L("全")));
  gtk_container_add (GTK_CONTAINER (hbox_row2), label_full);


  if (!pho_in_row1)
    gtk_box_pack_start (GTK_BOX (hbox_row2), event_box_pho, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(event_box_pho),"button-press-event",
                   G_CALLBACK(mouse_button_callback), NULL);


  label_pho = gtk_label_new(NULL);

  GtkWidget *frame_pho;
  if (gtab_in_area_button) {
	gtk_container_add (GTK_CONTAINER (event_box_pho), label_pho);
  }
  else {
	frame_pho = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_pho), GTK_SHADOW_OUT);
	gtk_container_add (GTK_CONTAINER (event_box_pho), frame_pho);
	gtk_container_set_border_width (GTK_CONTAINER (frame_pho), 0);
	gtk_container_add (GTK_CONTAINER (frame_pho), label_pho);
  }

  if (left_right_button_tips) {
#if GTK_CHECK_VERSION(2,12,0)
    gtk_widget_set_tooltip_text (event_box_pho, _(_L("左鍵符號，右鍵設定")));
#else
    GtkTooltips *button_gtab_tips = gtk_tooltips_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_gtab_tips), event_box_pho, _(_L("左鍵符號，右鍵設定")),NULL);
#endif
  }

  label_key_codes  = gtk_label_new(NULL);
  gtk_label_set_selectable(GTK_LABEL(label_key_codes), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox_row2), label_key_codes, FALSE, FALSE, 2);

  change_pho_font_size();

  gtk_widget_show_all (gwin_pho);

  gtk_widget_hide(label_key_codes);

  gtk_widget_hide(label_full);
}

void create_win_pho_gui()
{
  create_win_pho_gui_simple();

  if (pho_hide_row2) {
    gtk_widget_hide(hbox_row2);
    gtk_window_resize(GTK_WINDOW(gwin_pho), 1, 1);
  }

  current_hime_inner_frame = hime_inner_frame;
  current_pho_in_row1 = pho_in_row1;
}


gboolean pho_has_input();

void show_win_pho()
{
//  dbg("show_win_pho\n");
  create_win_pho();
  create_win_pho_gui();

  if (hime_pop_up_win && !pho_has_input())
    return;

#if UNIX
  if (!GTK_WIDGET_VISIBLE(gwin_pho))
#endif
  {
    gtk_widget_show(gwin_pho);
    move_win_pho(win_x, win_y);
  }

  gtk_widget_show(gwin_pho);
#if UNIX
  if (current_CS->b_raise_window)
#endif
    gtk_window_present(GTK_WINDOW(gwin_pho));

  show_win_sym();

  if (pho_hide_row2)
    gtk_widget_hide(hbox_row2);
  else
    gtk_widget_show(hbox_row2);
}


void hide_win_sym();

void hide_win_pho()
{
// dbg("hide_win_pho\n");
  if (!gwin_pho)
    return;

#if WIN32
  if (timeout_handle_pho) {
	  g_source_remove(timeout_handle_pho);
	  timeout_handle_pho = 0;
  }
#endif

  gtk_widget_hide(gwin_pho);
  hide_win_sym();
}


void init_tab_pho();
void get_win_gtab_geom();

void init_gtab_pho_query_win()
{
  init_tab_pho();
  get_win_gtab_geom();
  move_win_pho(win_x, win_y + win_yl);
}

char *get_full_str();

void win_pho_disp_half_full()
{
  if (hime_win_color_use)
     gtk_label_set_markup(GTK_LABEL(label_pho), get_full_str()); 
  else
     gtk_label_set_text(GTK_LABEL(label_pho), get_full_str());

  if (current_CS->im_state == HIME_STATE_CHINESE && (!current_CS->b_half_full_char))
    gtk_widget_hide(label_full);
  else
    gtk_widget_show(label_full);

  minimize_win_pho();
}

void get_win_pho_geom()
{
  if (!gwin_pho)
    return;

  gtk_window_get_position(GTK_WINDOW(gwin_pho), &win_x, &win_y);

  get_win_size(gwin_pho, &win_xl, &win_yl);
}


void change_pho_font_size()
{
  if (!top_bin)
    return;

  set_label_font_size(label_pho, hime_font_size_tsin_pho_in);

  set_label_font_size(label_pho_sele, hime_font_size);

  change_win_fg_bg(gwin_pho, label_pho_sele);
}

#if 0
void recreate_win_pho()
{
  gtk_widget_destroy(gwin_pho);
  top_bin = NULL;
  gwin_pho = NULL;
  create_win_pho();
  create_win_pho_gui_simple();
}
#endif
