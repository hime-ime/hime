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
#include "win-save-phrase.h"
#include "gtab.h"

extern int c_len;
extern gboolean test_mode;


typedef struct {
  WSP_S *mywsp;
  int mywspN;
  GtkWidget *label_countdown, *win;
  int countdown, countdown_handle;
} SAVE_SESS;

static void wsp_str(WSP_S *wsp, int wspN, char *out)
{
  int i;
  int ofs=0;

  for(i=0;i<wspN;i++) {
//    utf8_putchar(wsp[i].ch);
	int n = utf8cpy(out+ofs, wsp[i].ch);
	ofs+=n;
  }

//  out[ofs]=0;
//  dbg(" c_len:%d wsp %s\n", c_len, out);
}


static void free_mywsp(SAVE_SESS *sess)
{
  free(sess->mywsp); sess->mywsp=NULL;
  free(sess);
}


static gboolean close_win_save_phrase(GtkWidget *widget, gpointer data)
{
  SAVE_SESS *sess = (SAVE_SESS *)data;

  g_source_remove(sess->countdown_handle);
  gtk_widget_destroy(sess->win);
  free_mywsp(sess);
  return TRUE;
}

#if 0
static gint delete_event( GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data)
{
  free_mywsp(data);
  return FALSE;
}
#endif

extern int ph_key_sz;

static gboolean cb_ok(GtkWidget *widget, gpointer data)
{
  SAVE_SESS *sess = (SAVE_SESS *)data;
  g_source_remove(sess->countdown_handle);

  int i;
  phokey_t pho[MAX_PHRASE_LEN];
  u_int pho32[MAX_PHRASE_LEN];
  u_int64_t pho64[MAX_PHRASE_LEN];
  char tt[512];
  void *dat;
  wsp_str(sess->mywsp, sess->mywspN, tt);

  if (ph_key_sz==2) {
    for(i=0;i<sess->mywspN;i++)
      pho[i] = sess->mywsp[i].key;
    dat = pho;
  }
  else
  if (ph_key_sz==4) {
    for(i=0;i< sess->mywspN;i++) {
      pho32[i] = sess->mywsp[i].key;
    }
    dat = pho32;
  }
  else
  if (ph_key_sz==8) {
    for(i=0;i< sess->mywspN;i++)
      pho64[i] = sess->mywsp[i].key;
    dat = pho64;
  }

  save_phrase_to_db(dat, tt, sess->mywspN, 1);

  gtk_widget_destroy(sess->win);

  free_mywsp(sess);
  return TRUE;
}

static void disp_countdown(SAVE_SESS *sess)
{
  char tt[64];

  sprintf(tt, _(_L("%d 秒後自動加入")), sess->countdown);
  gtk_label_set_text(GTK_LABEL(sess->label_countdown), tt);
}


gboolean timeout_countdown(gpointer data)
{
  SAVE_SESS *sess = (SAVE_SESS *)data;

  if (!sess->countdown) {
    cb_ok(NULL, data);
    return FALSE;
  }

  sess->countdown--;
  disp_countdown(sess);
  return TRUE;
}


void create_win_save_phrase(WSP_S *wsp, int wspN)
{
#if WIN32
  if (test_mode)
    return;
#endif

  if (!wspN)
    return;

  SAVE_SESS *sess = tzmalloc(SAVE_SESS, 1);

  GtkWidget *main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(main_window), FALSE);
  sess->win = main_window;

  gtk_window_set_default_size(GTK_WINDOW (main_window), 20, 10);

  gtk_window_set_title(GTK_WINDOW(main_window), _(_L("加片語到詞庫")));

#if 0
  g_signal_connect (G_OBJECT (main_window), "delete_event",
                     G_CALLBACK (delete_event), sess);
#endif

  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (main_window), vbox);

  char tt[512];
  tt[0] = 0;
  wsp_str(wsp, wspN, tt);

  gtk_box_pack_start (GTK_BOX (vbox), gtk_label_new(tt), FALSE, FALSE, 0);

  int i;
  for(i=0; i<wspN; i++) {
    if (ph_key_sz==2)
      strcat(tt, phokey_to_str(wsp[i].key));
    strcat(tt, " ");
  }

  if (tt[0])
    gtk_box_pack_start (GTK_BOX (vbox), gtk_label_new(tt), FALSE, FALSE, 0);

  sess->mywsp = tmemdup(wsp, WSP_S, wspN);
  sess->mywspN = wspN;

  GtkWidget *hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_cancel_ok , FALSE, FALSE, 5);

  GtkWidget *button_ok = gtk_button_new_from_stock (GTK_STOCK_OK);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_ok, TRUE, TRUE, 5);

  GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);

  sess->label_countdown = gtk_label_new(NULL);
  gtk_box_pack_start (GTK_BOX (vbox), sess->label_countdown, FALSE, FALSE, 5);

#if 1
#if WIN32
  set_no_focus(main_window);
#endif
  gtk_widget_realize(main_window);
#if UNIX
  set_no_focus(main_window);
#else
  win32_init_win(main_window);
#endif
#endif

//  dbg("mmmmmmmmmmmmm\n");

  GTK_WIDGET_SET_FLAGS (button_ok, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button_ok);


#if 1
//  dbg("main_window %x\n", main_window);
  g_signal_connect (G_OBJECT (button_cancel), "clicked",
                            G_CALLBACK (close_win_save_phrase),
                            sess);

  g_signal_connect (G_OBJECT (button_ok), "clicked",
                            G_CALLBACK (cb_ok),
                            sess);
#endif

  gtk_window_present(GTK_WINDOW(main_window));
  gtk_window_set_keep_above(GTK_WINDOW(main_window), TRUE);
//  gtk_window_set_modal(GTK_WINDOW(main_window), TRUE);

  sess->countdown = 3;
  disp_countdown(sess);
  sess->countdown_handle = g_timeout_add(1000, timeout_countdown, sess);
  gtk_widget_show_all(main_window);
}
