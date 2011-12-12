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
#include "config.h"
#if HIME_i18n_message
#include <libintl.h>
#endif
#include "gst.h"
#include "tsin.h"


GtkWidget *hbox_buttons;
char current_str[MAX_PHRASE_LEN*CH_SZ+1];

gboolean b_pinyin, is_chs;

static GtkClipboard *pclipboard;

GtkWidget *mainwin;
GtkTextBuffer *buffer;


void do_exit()
{
  exit(0);
}


#define MAX_SAME_PHO 32

void all_wrap()
{
  GtkTextIter mstart,mend;

  gtk_text_buffer_get_bounds (buffer, &mstart, &mend);
  gtk_text_buffer_apply_tag_by_name (buffer, "char_wrap", &mstart, &mend);
}

char *phokey2pinyin(phokey_t k);

static void selection_received(GtkClipboard *pclip, const gchar *text, gpointer data)
{
  if (!text)
    return;

  char *outtext = NULL;
  int outlen = 0;

  while (*text) {
    char tt[CH_SZ * 5 * MAX_SAME_PHO + 1];
    int len = u8cpy(tt, (char *)text);
    tt[len]=0;

    phokey_t phokeys[MAX_SAME_PHO];
    int phokeysN = utf8_pho_keys((char *)text, phokeys);

    int i;
    for(i=0; i <phokeysN ;i++) {
      char *phostr;

      if (b_pinyin)
        phostr = phokey2pinyin(phokeys[i]);
      else
        phostr = phokey_to_str(phokeys[i]);

      strcat(tt, phostr);
    }

    int ttlen = strlen(tt);
    outtext = (char *)realloc(outtext, outlen + ttlen + 1);

    memcpy(outtext + outlen, tt, ttlen);
    outlen+=ttlen;

    text += len;
  }

  outtext[outlen]=0;

  gtk_text_buffer_set_text (buffer, outtext, -1);
  free(outtext);

  all_wrap();
}

void req_clipboard()
{
  gtk_clipboard_request_text(pclipboard, selection_received, mainwin);
}


gboolean cb_button_fetch()
{
  req_clipboard();
  return TRUE;
}

void set_window_hime_icon(GtkWidget *window);

#if WIN32
void init_hime_program_files();
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
void init_TableDir();
#endif

gboolean is_pinyin_kbm();

int main(int argc, char **argv)
{
#if WIN32
  init_TableDir();
#endif

  gtk_init (&argc, &argv);

#if HIME_i18n_message
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif

  b_pinyin = is_pinyin_kbm();

  pho_load();

  mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(mainwin), FALSE);
  gtk_window_set_default_size(GTK_WINDOW (mainwin), 640, 220);
  set_window_hime_icon(mainwin);

  GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_top), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER(mainwin), vbox_top);

  GtkWidget *view = gtk_text_view_new ();
  gtk_widget_set_hexpand (view, TRUE);
  gtk_widget_set_vexpand (view, TRUE);
  gtk_container_add (GTK_CONTAINER(sw), view);

  gtk_box_pack_start (GTK_BOX (vbox_top), sw, TRUE, TRUE, 0);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

  gtk_text_buffer_create_tag (buffer,
     "blue_background", "background", "blue", "foreground", "white", NULL);

  gtk_text_buffer_create_tag (buffer, "char_wrap",
			      "wrap_mode", GTK_WRAP_CHAR, NULL);

  hbox_buttons = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_buttons, FALSE, FALSE, 0);

  GtkWidget *button_fetch = gtk_button_new_with_label(_(_L("自剪貼區更新")));
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_fetch, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_fetch), "clicked",
     G_CALLBACK (cb_button_fetch), NULL);

  GtkWidget *button_exit = gtk_button_new_from_stock(GTK_STOCK_QUIT);
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_exit, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_exit), "clicked",
     G_CALLBACK (do_exit), NULL);


  g_signal_connect (G_OBJECT (mainwin), "delete_event",
                    G_CALLBACK (do_exit), NULL);

  gtk_widget_show_all(mainwin);
#if UNIX
  pclipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
#else
  pclipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
#endif

  req_clipboard();

  gtk_main();
  return 0;
}
