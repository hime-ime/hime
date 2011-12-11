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
#include "lang.h"
#include "tsin.h"
#include "gtab.h"

extern char *current_tsin_fname;
typedef unsigned int u_int32_t;

void init_TableDir();
void init_gtab(int inmdno);
gboolean init_tsin_table_fname(INMD *p, char *fname);
void load_tsin_db0(char *infname, gboolean is_gtab_i);

INMD *pinmd;
char gtab_tsin_fname[256];
char is_gtab;

char *phokey2pinyin(phokey_t k);
gboolean b_pinyin;
GtkWidget *hbox_buttons;
char current_str[MAX_PHRASE_LEN*CH_SZ+1];
extern gboolean is_chs;

GtkWidget *mainwin;
GtkTextBuffer *buffer;

static char **phrase;
static int phraseN=0;


void cp_ph_key(void *in, int idx, void *dest)
{
  if (ph_key_sz==2) {
    phokey_t *pharr = (phokey_t *)in;
    in = &pharr[idx];
  } else
  if (ph_key_sz==4) {
    u_int32_t *pharr4 = (u_int32_t *)in;
    in = &pharr4[idx];
  } else {
    u_int64_t *pharr8 = (u_int64_t *)in;
    in = &pharr8[idx];
  }

  memcpy(dest, in, ph_key_sz);
}

void *get_ph_key_ptr(void *in, int idx)
{
  if (ph_key_sz==2) {
    phokey_t *pharr = (phokey_t *)in;
    return &pharr[idx];
  } else
  if (ph_key_sz==4) {
    u_int32_t *pharr4 = (u_int32_t *)in;
    return &pharr4[idx];
  } else {
    u_int64_t *pharr8 = (u_int64_t *)in;
    return &pharr8[idx];
  }
}

int lookup_gtab_key(char *ch, void *out)
{
  int outN=0;
  INMD *tinmd = &inmd[default_input_method];

  int i;
  for(i=0; i < tinmd->DefChars; i++) {
    char *chi = (char *)tblch2(tinmd, i);

    if (!(chi[0] & 0x80))
      continue;
    if (!utf8_eq(chi, ch))
      continue;

    u_int64_t key = CONVT2(tinmd, i);
    if (ph_key_sz==4) {
      u_int32_t key32 = (u_int32_t)key;
      memcpy(get_ph_key_ptr(out, outN), &key32, ph_key_sz);
    } else
      memcpy(get_ph_key_ptr(out, outN), &key, ph_key_sz);
    outN++;
  }

  return outN;
}


static int qcmp_str(const void *aa, const void *bb)
{
  char *a = * (char **)aa, *b = * (char **)bb;

  return strcmp(a,b);
}

extern FILE *fph;

void load_ts_phrase()
{
  FILE *fp = fph;

  int i;
  for(i=0; i < phraseN; i++)
    free(phrase[i]);
  free(phrase); phrase = NULL;
  phraseN = 0;

  dbg("fname %s\n", current_tsin_fname);

  int ofs = is_gtab ? sizeof(TSIN_GTAB_HEAD):0;
  fseek(fp, ofs, SEEK_SET);

  while (!feof(fp)) {
    u_int64_t phbuf[MAX_PHRASE_LEN];
    char chbuf[MAX_PHRASE_LEN * CH_SZ + 1];
    u_char clen;
    usecount_t usecount;

    clen = 0;

    fread(&clen,1,1,fp);

    if (clen > MAX_PHRASE_LEN)
      p_err("bad tsin db clen %d > MAX_PHRASE_LEN %d\n", clen, MAX_PHRASE_LEN);

    fread(&usecount,sizeof(usecount_t), 1, fp);
    fread(phbuf, ph_key_sz, clen, fp);
    int tlen = 0;

    for(i=0; i < clen; i++) {
      int n = fread(&chbuf[tlen], 1, 1, fp);
      if (n<=0)
        goto stop;
      int len=utf8_sz(&chbuf[tlen]);
      fread(&chbuf[tlen+1], 1, len-1, fp);
      tlen+=len;
    }

    if (clen < 2)
      continue;

    chbuf[tlen]=0;
    phrase = trealloc(phrase, char *, phraseN+1);

    phrase[phraseN++] = strdup(chbuf);
  }


stop:

//  fclose(fp);

  qsort(phrase, phraseN, sizeof(char *), qcmp_str);

  dbg("phraseN: %d\n", phraseN);
}

gboolean pharse_search(char *s)
{
  return bsearch(&s, phrase, phraseN, sizeof(char *), qcmp_str) != NULL;
}

void all_wrap()
{
  GtkTextIter mstart,mend;

  gtk_text_buffer_get_bounds (buffer, &mstart, &mend);
  gtk_text_buffer_apply_tag_by_name (buffer, "char_wrap", &mstart, &mend);
}


static void cb_button_parse(GtkButton *button, gpointer user_data)
{
  int char_count = gtk_text_buffer_get_char_count (buffer);

  int i;

  load_ts_phrase();

  char_count = gtk_text_buffer_get_char_count (buffer);

  all_wrap();

  dbg("parse char_count:%d\n", char_count);

  for(i=0; i < char_count; ) {
    int len;

    for(len=MAX_PHRASE_LEN; len>=2 ; len--) {
      u_char txt[MAX_PHRASE_LEN*CH_SZ + 1];
      int txtN=0, u8chN=0;

      gboolean b_ignore = FALSE;
      int k;
      for(k=0; k<len && i+k < char_count; k++) {
        GtkTextIter start,end;
        gtk_text_buffer_get_iter_at_offset (buffer, &start, i+k);
        gtk_text_buffer_get_iter_at_offset (buffer, &end, i+k+1);
        char *utf8 = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

        if (!(utf8[0] & 128))
          b_ignore = TRUE;

        int wn = strlen(utf8);

        memcpy(&txt[txtN], utf8, wn);

        txtN+= wn;
        u8chN++;
      }

      if (b_ignore || txtN < 2)
        continue;

      txt[txtN] = 0;
//      dbg("try len:%d txtN:%d %s\n", len, txtN, txt);
      if (!pharse_search((char *)txt))
        continue;

//      dbg("match .... %d %d\n", i, len);

      GtkTextIter mstart,mend;

      gtk_text_buffer_get_iter_at_offset (buffer, &mstart, i);
      gtk_text_buffer_get_iter_at_offset (buffer, &mend, i+len);
      gtk_text_buffer_apply_tag_by_name (buffer, "blue_background", &mstart, &mend);
#if 1
      // why do I have to repeat this
      gtk_text_buffer_get_iter_at_offset (buffer, &mstart, i);
      gtk_text_buffer_get_iter_at_offset (buffer, &mend, i+len);
      gtk_text_buffer_apply_tag_by_name (buffer, "blue_background", &mstart, &mend);
#endif
      gdk_flush();
    }

    i+=len;
  }
}

#define MAX_SAME_CHAR_PHO (16)

typedef struct {
  u_int64_t phokeys[MAX_SAME_CHAR_PHO];
  int phokeysN;
  GtkWidget *opt_menu;
} char_pho;



static char_pho bigpho[MAX_PHRASE_LEN];
static int bigphoN;

static GtkWidget *hbox_pho_sel;

void destroy_pho_sel_area()
{
  gtk_widget_destroy(hbox_pho_sel);
}

static void cb_button_ok(GtkButton *button, gpointer user_data)
{
  u_int64_t pharr8[MAX_PHRASE_LEN];

  int i;
  for(i=0; i < bigphoN; i++) {
    int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(bigpho[i].opt_menu));
    void *dest = get_ph_key_ptr(pharr8, i);

    cp_ph_key(bigpho[i].phokeys, idx, dest);
  }

  save_phrase_to_db(pharr8, current_str, bigphoN, 0);

  destroy_pho_sel_area();

  GtkTextMark *selebound =  gtk_text_buffer_get_selection_bound(buffer);
  gtk_text_mark_set_visible(selebound, FALSE);

  cb_button_parse(NULL, NULL);

}

static void cb_button_cancel(GtkButton *button, gpointer user_data)
{
  destroy_pho_sel_area();
}

int gtab_key2name(INMD *tinmd, u_int64_t key, char *t, int *rtlen);
GtkWidget *create_pho_sel_area()
{
  hbox_pho_sel = gtk_hbox_new (FALSE, 0);

  int i;

  for(i=0; i < bigphoN; i++) {
    bigpho[i].opt_menu = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
    GtkWidget *menu = gtk_menu_new ();
#endif
    gtk_box_pack_start (GTK_BOX (hbox_pho_sel), bigpho[i].opt_menu, FALSE, FALSE, 0);

    int j;
    for(j=0; j < bigpho[i].phokeysN; j++) {
      char t[128];
      char *phostr;

      if (is_gtab) {
        int tlen;
        u_int64_t key64;
        if (ph_key_sz == 4) {
          u_int32_t key32;
          cp_ph_key(bigpho[i].phokeys,j, &key32);
          key64 = key32;
        } else
          cp_ph_key(bigpho[i].phokeys,j, &key64);

        gtab_key2name(pinmd, key64, t, &tlen);
//        dbg("%d,%d] %s\n", i,j, t);
        phostr = t;
      } else {
        phokey_t k;
        cp_ph_key(bigpho[i].phokeys, j, &k);
        phostr = b_pinyin?
        phokey2pinyin(k):phokey_to_str(k);
      }

#if GTK_CHECK_VERSION(2,4,0)
      gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (bigpho[i].opt_menu), phostr);
#else
      GtkWidget *item = gtk_menu_item_new_with_label (phostr);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
#endif
    }

#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_set_active (GTK_COMBO_BOX(bigpho[i].opt_menu), 0);
#else
    gtk_option_menu_set_menu (GTK_OPTION_MENU (bigpho[i].opt_menu), menu);
#endif

  }


  GtkWidget *button_ok = gtk_button_new_with_label("OK to add");
  gtk_box_pack_start (GTK_BOX (hbox_pho_sel), button_ok, FALSE, FALSE, 20);
  g_signal_connect (G_OBJECT (button_ok), "clicked",
     G_CALLBACK (cb_button_ok), NULL);

  GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox_pho_sel), button_cancel, FALSE, FALSE, 20);
  g_signal_connect (G_OBJECT (button_cancel), "clicked",
     G_CALLBACK (cb_button_cancel), NULL);


  return hbox_pho_sel;
}


static void cb_button_add(GtkButton *button, gpointer user_data)
{
  GtkTextIter start, end;

  if (!gtk_text_buffer_get_selection_bounds(buffer, &start, &end))
    return;

  char *utf8 = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  strcpy(current_str, utf8);

  g_free(utf8);

  bigphoN = 0;
  char *p = current_str;
  while (*p) {
    char_pho *pbigpho = &bigpho[bigphoN++];

    if (ph_key_sz==2) {
      pbigpho->phokeysN = utf8_pho_keys(p, (phokey_t*)pbigpho->phokeys);
    } else {
      pbigpho->phokeysN = lookup_gtab_key(p, pbigpho->phokeys);
    }

    p+=utf8_sz(p);

    if (!pbigpho->phokeysN) {
      dbg(" no mapping to pho\n");
      return;
    }
  }

  GtkWidget *sel =  create_pho_sel_area();
  gtk_box_pack_start (GTK_BOX (hbox_buttons), sel, FALSE, FALSE, 20);

  gtk_widget_show_all(hbox_buttons);

}

Display *dpy;

void do_exit()
{
  send_hime_message(
#if UNIX
	  dpy,
#endif
	  RELOAD_TSIN_DB);

  exit(0);
}

void load_tsin_db();
void set_window_hime_icon(GtkWidget *window);
#if WIN32
void init_hime_program_files();
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

gboolean is_pinyin_kbm();

int main(int argc, char **argv)
{
  init_TableDir();
  set_is_chs();
  b_pinyin = is_pinyin_kbm();

  gtk_init (&argc, &argv);
  load_setttings();
  load_gtab_list(TRUE);


#if HIME_i18n_message
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif

  pinmd = &inmd[default_input_method];

  if (pinmd->method_type == method_type_TSIN) {
    dbg("is tsin\n");
    pho_load();
    load_tsin_db();
    ph_key_sz = 2;
  } else
  if (pinmd->filename) {
    dbg("gtab filename %s\n", pinmd->filename);
    init_gtab(default_input_method);
    is_gtab = TRUE;
    init_tsin_table_fname(pinmd, gtab_tsin_fname);
    load_tsin_db0(gtab_tsin_fname, TRUE);
  } else
    p_err("Your default input method %s doesn't use phrase database",
      pinmd->cname);

  dbg("ph_key_sz: %d\n", ph_key_sz);

#if UNIX
  dpy = GDK_DISPLAY();
#endif

  mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(mainwin), FALSE);
  gtk_window_set_default_size(GTK_WINDOW (mainwin), 640, 520);
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

#if UNIX
  char *text = _(_L("按滑鼠中鍵, 貼上你要 hime-tslearn 學習的文章。"));
#else
  char *text = _(_L("按 ctrl-V, 貼上你要 hime-tslearn 學習的文章。"));
#endif

  gtk_text_buffer_set_text (buffer, text, -1);

  gtk_text_buffer_create_tag (buffer,
     "blue_background", "background", "blue", "foreground", "white", NULL);

  gtk_text_buffer_create_tag (buffer, "char_wrap",
			      "wrap_mode", GTK_WRAP_CHAR, NULL);

  hbox_buttons = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_buttons, FALSE, FALSE, 0);

  GtkWidget *button_parse = gtk_button_new_with_label(_(_L("標示已知詞")));
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_parse, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_parse), "clicked",
     G_CALLBACK (cb_button_parse), NULL);

  GtkWidget *button_add = gtk_button_new_with_label(_(_L("新增詞")));
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_add, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_add), "clicked",
     G_CALLBACK (cb_button_add), NULL);


  GtkWidget *button_quit = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_quit, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_quit), "clicked",
     G_CALLBACK (do_exit), NULL);


  g_signal_connect (G_OBJECT (mainwin), "delete_event",
                    G_CALLBACK (do_exit), NULL);

  all_wrap();

  gtk_widget_show_all(mainwin);
#if WIN32
  gtk_window_present(GTK_WINDOW(mainwin));
#endif

  gtk_main();
  return 0;
}
