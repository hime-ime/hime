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
#include "win-sym.h"
#include "gst.h"
#include "tsin.h"


static GtkWidget *phrase_save_win;

void destroy_phrase_save_menu()
{
  if (!phrase_save_win)
    return;
  gtk_widget_destroy(phrase_save_win);
  phrase_save_win = NULL;
}

void save_phrase(int save_frm, int len);
static void cb_clicked(GtkWidget *widget, gpointer data)
{
  destroy_phrase_save_menu();

  int v = GPOINTER_TO_INT(data);
  int len = v & 0xff;
  int idx = v >> 8;

  if (!len)
    return;

  save_phrase(idx, len);
}

void chpho_extract(CHPHO *chph, int len, phokey_t *pho, char *ch);

static void add_button(GtkWidget *vbox, char *s, int idx, int len)
{
  int data = (int)((idx << 8) | len);

  GtkWidget *but = gtk_button_new_with_label(s);
  gtk_box_pack_start (GTK_BOX (vbox), but, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (but), "clicked",  G_CALLBACK (cb_clicked), GINT_TO_POINTER(data));
}

void create_phrase_save_menu(GdkEventButton * event)
{
  destroy_phrase_save_menu();

  int len = tss.c_len - tss.c_idx;
  if ((len < 2 || len > MAX_PHRASE_LEN) && tss.c_idx < 2)
    return;

  phrase_save_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(phrase_save_win), FALSE);
#if WIN32
  set_no_focus(phrase_save_win);
#endif
  gtk_widget_realize (phrase_save_win);

#if UNIX
  set_no_focus(phrase_save_win);
#else
  win32_init_win(phrase_save_win);
#endif
  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
  gtk_container_add(GTK_CONTAINER(phrase_save_win), vbox);
  char tt[512];

  add_button(vbox, _(_L("關閉")), 0, 0);

  GtkWidget *label = gtk_label_new(_(_L("--- 加到詞庫 ---")));
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);


  if (len > 1 && len < MAX_PHRASE_LEN) {
    phokey_t phs[MAX_PHRASE_LEN];
    char str[MAX_PHRASE_STR_LEN];
    chpho_extract(&tss.chpho[tss.c_idx], len, phs, str);
    sprintf(tt, _(_L("%s shift-Enter")), str);
    add_button(vbox, tt, tss.c_idx, len);
  }

  if (tss.c_idx > 1) {
    for(len=2; len <= tss.c_idx && len <= 5; len++) {
      phokey_t phs[MAX_PHRASE_LEN];
      char str[MAX_PHRASE_STR_LEN];
      chpho_extract(&tss.chpho[tss.c_idx - len], len, phs, str);
      sprintf(tt, _(_L("%s Ctrl-%d")), str, len);
      add_button(vbox, tt, tss.c_idx, len);
    }
  }

  gtk_widget_show_all(phrase_save_win);
  int x, y, w_xl, w_yl;

  get_win_size(phrase_save_win, &w_xl, &w_yl);
  x = win_x;
  if (x + w_xl > dpy_xl)
    x = dpy_xl - w_xl;

  y = win_y + win_yl;
  if (y + w_yl > dpy_yl)
    y = win_y - w_yl;

  gtk_window_move(GTK_WINDOW(phrase_save_win), x, y);
}
