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

#include <sys/stat.h>
#include "hime.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"

static GtkWidget *gwin_pho_near = NULL;

static struct {
  int type;
  char group[5];
} groups[] = {
  {0, {15, 19, -1}},   // ㄓㄗ, 以 -1 做結尾
  {0, {16, 20, -1}},    // ㄔㄘ
  {0, {8, 18, -1}},     // ㄌㄖ
  {0, {4, 11,-1}},     // ㄈㄏ
  {0, {17, 21,-1}},   // ㄕㄙ
  {2, {2, 3, -1}},     // ㄛㄜ
  {2, {10, 12, -1}},   // ㄣㄥ
};

static int groupsN=sizeof(groups)/ sizeof(groups[0]);

static char *find_group(int type, int num)
{
  int i;

  for(i=0; i < groupsN; i++) {
    if (groups[i].type != type)
      continue;

    char *p;
    for (p = groups[i].group; *p > 0; p++)
      if (*p == num) {
        return groups[i].group;
      }
  }

  static char alone[2]={-1, -1};
  alone[0] = num;

  return alone;
}

void key_typ_pho(phokey_t phokey, u_char rtyp_pho[]);
gboolean get_start_stop_idx(phokey_t key, int *start_i, int *stop_i);
void close_win_pho_near();

typedef struct {
  GtkWidget *label;
  phokey_t pk;
} NEAR_ENTRY;

static NEAR_ENTRY *near_entries;
static int near_entriesN;
gboolean add_to_tsin_buf(char *str, phokey_t *pho, int len);
void tsin_remove_last();

static void cb_sel (GtkWidget *button, gpointer user_data)
{
  NEAR_ENTRY *near_entry = &near_entries[GPOINTER_TO_INT(user_data)];
  const char *ch = gtk_label_get_text(GTK_LABEL(near_entry->label));
  char tt[CH_SZ+1];
  strcpy(tt, ch);

  tsin_remove_last();
  char_play(tt);
  add_to_tsin_buf(tt, &near_entry->pk, 1);

  close_win_pho_near();
}

char *phokey2pinyin(phokey_t k);

void create_win_pho_near(phokey_t pho)
{
  if (gwin_pho_near)
    close_win_pho_near();

  gwin_pho_near = gtk_window_new (GTK_WINDOW_TOPLEVEL);
gtk_window_set_has_resize_grip(GTK_WINDOW(gwin_pho_near), FALSE);
#if WIN32
  set_no_focus(gwin_pho_near);
#endif
  gtk_widget_realize (gwin_pho_near);
#if UNIX
  set_no_focus(gwin_pho_near);
#else
  win32_init_win(gwin_pho_near);
#endif

  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_container_add(GTK_CONTAINER (gwin_pho_near), frame);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_top), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (frame), vbox_top);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_top), 0);

  char t_typ_pho[4], mtyp_pho[4];
  key_typ_pho(pho, (u_char *)t_typ_pho);
  memcpy(mtyp_pho, t_typ_pho, 4);

  char *group0;
  char *group2;
  gboolean b_first = TRUE;

//  dbg("zz %d %d\n",t_typ_pho[0], t_typ_pho[2]);

  for (group0=find_group(0, t_typ_pho[0]); *group0 >=0 ; group0++) {
    mtyp_pho[0] = *group0;
//    dbg("%d\n",mtyp_pho[0]);

    for (group2=find_group(2, t_typ_pho[2]); *group2 >=0; group2++) {
      mtyp_pho[2] = *group2;

      for (mtyp_pho[3]=0;  mtyp_pho[3]< 5; mtyp_pho[3]++) {
//      dbg("  %d\n",mtyp_pho[2]);
        phokey_t pk = pho2key(mtyp_pho);
        char *pho_str = pin_juyin ?
        phokey2pinyin(pk):phokey_to_str(pk);

        int start_i, stop_i;

        if (!get_start_stop_idx(pk, &start_i, &stop_i))
          continue;

        if (b_first) {
          b_first = FALSE;
        } else {
          GtkWidget *separator = gtk_hseparator_new ();
          gtk_box_pack_start (GTK_BOX (vbox_top), separator, FALSE, FALSE, 0);
        }

        GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vbox_top), hbox, FALSE, FALSE, 0);

        GtkWidget *label_pho = gtk_label_new(pho_str);
        gtk_box_pack_start (GTK_BOX (hbox), label_pho, FALSE, FALSE, 0);
        set_label_font_size(label_pho, hime_font_size_pho_near);


        int i;
        for(i=start_i; i<stop_i; i++) {
          char tt[CH_SZ+1];
          bzero(tt, sizeof(tt));
          utf8cpy(tt, pho_idx_str(i));

          GtkWidget *button = gtk_button_new();
          gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

          GtkWidget *label_pho = gtk_label_new(tt);
          gtk_container_add (GTK_CONTAINER (button), label_pho);
          set_label_font_size(label_pho, hime_font_size_pho_near);

          near_entries = trealloc(near_entries, NEAR_ENTRY, near_entriesN);
          near_entries[near_entriesN].label = label_pho;
          near_entries[near_entriesN].pk = pk;
          g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (cb_sel), GINT_TO_POINTER(near_entriesN));

          near_entriesN++;
        }
      }
    }
  }

  gtk_widget_show_all(gwin_pho_near);
}

void close_win_pho_near()
{
  if (!gwin_pho_near)
    return;

  gtk_widget_destroy(gwin_pho_near);
  gwin_pho_near = NULL;

  free(near_entries);
  near_entriesN = 0;
  near_entries = NULL;
}
