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
#include "gtab-buf.h"

static GtkWidget *gwin_sym = NULL;
static int cur_in_method;
gboolean win_sym_enabled=0;

typedef struct {
  char **sym;
  int symN;
} SYM_ROW;

static SYM_ROW *syms;
static int symsN;

typedef struct {
  SYM_ROW *syms;
  int symsN;
} PAGE;

static PAGE *pages;
static int pagesN;
static int idx;

extern char *TableDir;

void get_hime_user_or_sys_fname(char *name, char fname[]);

FILE *watch_fopen(char *filename, time_t *pfile_modify_time)
{
  FILE *fp;
  char fname[256];

  get_hime_user_or_sys_fname(filename, fname);

  if ((fp=fopen(fname, "rb"))==NULL) {
#if UNIX
    strcat(strcat(strcpy(fname, TableDir), "/"), filename);
#else
    strcat(strcat(strcpy(fname, TableDir), "\\"), filename);
#endif

    if ((fp=fopen(fname, "rb"))==NULL)
     return NULL;
  }

#if UNIX
  struct stat st;
  fstat(fileno(fp), &st);
#else
  struct _stat st;
  _fstat(fileno(fp), &st);
#endif

  if (st.st_mtime == *pfile_modify_time) {
    fclose(fp);
    return NULL;
  }

  *pfile_modify_time = st.st_mtime;
  return fp;
}

static void save_page()
{
  if (!symsN)
    return;

  pages=trealloc(pages, PAGE, pagesN+1);
  pages[pagesN].syms = syms;
  pages[pagesN].symsN = symsN;
  pagesN++;
  syms = NULL;
  symsN = 0;
}


static gboolean read_syms()
{
  FILE *fp;
  static char symbol_table[] = "symbol-table";
  static time_t file_modify_time;

  if ((fp=watch_fopen(symbol_table, &file_modify_time))==NULL)
    return FALSE;

  skip_utf8_sigature(fp);

  int pg;
  for(pg=0; pg < pagesN; pg++) {
    syms = pages[pg].syms;
    symsN = pages[pg].symsN;

    int i;
    for(i=0; i < symsN; i++) {
      int j;
      for(j=0; j < syms[i].symN; j++)
        if (syms[i].sym[j])
          free(syms[i].sym[j]);
    }
    free(syms);
  }
  pagesN = 0; pages = NULL;
  syms = NULL; symsN = 0;


  while (!feof(fp)) {
    char tt[1024];

    bzero(tt, sizeof(tt));
    myfgets(tt, sizeof(tt), fp);
//    dbg("%d] %s\n",strlen(tt), tt);

#if 0
    int len=strlen(tt);
    if (!len)
      continue;

    if (tt[len-1]=='\n') {
      tt[len-1]=0;
    }
#endif

    if (tt[0]==0)
      save_page();

    if (tt[0]=='#')
      continue;

    char *p=tt;

    syms=trealloc(syms, SYM_ROW, symsN+1);
    SYM_ROW *psym = &syms[symsN++];
    bzero(psym, sizeof(SYM_ROW));


    while (*p) {
      char *n = p;

      while (*n && *n!='\t')
        n++;

      *n = 0;

      psym->sym=trealloc(psym->sym, char *, psym->symN+1);
      psym->sym[psym->symN++] = strdup(p);

      p = n + 1;
    }

    if (!psym->symN) {
      free(syms);
      syms=NULL;
      symsN=0;
    }
  }

  if (symsN)
    save_page();

  fclose(fp);

  idx = 0;
  syms = pages[idx].syms;
  symsN = pages[idx].symsN;

  return TRUE;
}


gboolean add_to_tsin_buf(char *str, phokey_t *pho, int len);
void send_text_call_back(char *text);
void tsin_reset_in_pho(), reset_gtab_all(), clr_in_area_pho();
void force_preedit_shift();
gboolean output_gbuf();
void output_buffer_call_back();
gboolean gtab_cursor_end(),gtab_phrase_on();
void flush_tsin_buffer();
gboolean tsin_cursor_end();
void add_to_tsin_buf_str(char *str);

extern int c_len;
extern short gbufN;
static void cb_button_sym(GtkButton *button, GtkWidget *label)
{
//  dbg("cb_button_sym\n");
  char *str = (char *) gtk_label_get_text(GTK_LABEL(label));

#if USE_TSIN
  if (current_method_type() == method_type_TSIN && current_CS->im_state == HIME_STATE_CHINESE) {
    add_to_tsin_buf_str(str);
    if (tsin_cursor_end()) {
      flush_tsin_buffer();
      output_buffer_call_back();
    } else {
      force_preedit_shift();
    }
  }
  else
#endif
  if (gtab_phrase_on()) {
    insert_gbuf_nokey(str);
    if (gtab_cursor_end()) {
      output_gbuf();
      output_buffer_call_back();
    } else
      force_preedit_shift();
  } else {
    send_text_call_back(str);
  }

  switch (current_method_type()) {
    case method_type_PHO:
       clr_in_area_pho();
       break;
#if USE_TSIN
    case method_type_TSIN:
       tsin_reset_in_pho();
       break;
#endif
    case method_type_MODULE:
       break;
    default:
       reset_gtab_all();
       break;
  }

  if (hime_win_sym_click_close) {
    win_sym_enabled=0;
    hide_win_sym();
  }
}

void update_active_in_win_geom();
extern int win_status_y;

void move_win_sym()
{
#if 0
  dbg("move_win_sym %d\n", current_CS->in_method);
#endif
  if (!gwin_sym)
    return;

  int wx, wy;
#if 0
  if (hime_pop_up_win) {
    wx = dpy_xl;
  } else
#endif
  {
  //  dbg("win_y: %d  %d\n", win_y, win_yl);
    update_active_in_win_geom();

    wx = win_x; wy = win_y + win_yl;
  }

  int winsym_xl, winsym_yl;
  get_win_size(gwin_sym, &winsym_xl, &winsym_yl);

  if (wx + winsym_xl > dpy_xl)
    wx = dpy_xl - winsym_xl;
  if (wx < 0)
    wx = 0;

#if 0
  if (hime_pop_up_win) {
    wy = win_status_y - winsym_yl;
  } else
#endif
  {
    if (wy + winsym_yl > dpy_yl)
      wy = win_y - winsym_yl;
    if (wy < 0)
      wy = 0;
  }

  gtk_window_move(GTK_WINDOW(gwin_sym), wx, wy);
}


void hide_win_sym()
{
  if (!gwin_sym)
    return;
  gtk_widget_hide(gwin_sym);

}

void show_win_sym()
{
  if (!current_CS)
    return;

  if (!gwin_sym || !win_sym_enabled || current_CS->im_state == HIME_STATE_DISABLED)
    return;
#if 0
  dbg("show_win_sym\n");
#endif
  gtk_widget_show_all(gwin_sym);
  move_win_sym();
#if WIN32
  gtk_window_present(GTK_WINDOW(gwin_sym));
#endif
}


void lookup_gtab_out(char *ch, char *out);
void str_to_all_phokey_chars(char *b5_str, char *out);

static void sym_lookup_key(char *instr, char *outstr)
{
  if (current_method_type() == method_type_PHO || current_method_type() == method_type_TSIN) {
    str_to_all_phokey_chars(instr, outstr);
  } else {
    outstr[0]=0;

    while (*instr) {
      char tt[512];
      tt[0]=0;
      lookup_gtab_out(instr, tt);
      strcat(outstr, tt);

      instr+= utf8_sz(instr);

      if (*instr)
          strcat(outstr, " | ");
    }
  }
}

static void destory_win()
{
  if (gwin_sym)
    gtk_widget_destroy(gwin_sym);
  gwin_sym = NULL;
}

static void disp_win_sym()
{
  syms = pages[idx].syms;
  symsN = pages[idx].symsN;
  destory_win();
//  win_sym_enabled = 0;
  create_win_sym();
#if WIN32
  show_win_sym();
#endif
}

gboolean win_sym_page_up()
{
  if (!win_sym_enabled)
	  return FALSE;
  idx--;
  if (idx < 0)
  idx = pagesN - 1;
  disp_win_sym();
  return TRUE;
}

gboolean win_sym_page_down()
{
//  dbg("win_sym_page_down\n");
  if (!win_sym_enabled)
    return FALSE;
  idx = (idx+1) % pagesN;
  disp_win_sym();
  return TRUE;
}


static gboolean button_scroll_event(GtkWidget *widget,GdkEventScroll *event, gpointer user_data)
{
  if (pagesN < 2)
    return TRUE;

  switch (event->direction) {
    case GDK_SCROLL_UP:
	  win_sym_page_up();
      break;
    case GDK_SCROLL_DOWN:
	  win_sym_page_down();
      break;
    default:
      break;
  }

  return TRUE;
}



static void mouse_button_callback_up_down( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
	GdkEventScroll sc;
	sc.direction = data ? GDK_SCROLL_UP : GDK_SCROLL_DOWN;
	button_scroll_event(NULL, &sc, NULL);
}

void create_win_sym()
{
  if (!current_CS) {
    dbg("create_win_sym, null CS\n");
    return;
  }

  if (current_CS->in_method < 0) {
    p_err("bad current_CS %d\n", current_CS->in_method);
  }

  if (current_method_type() != method_type_PHO && current_method_type() != method_type_TSIN && current_method_type() != method_type_MODULE && !cur_inmd)
    return;

  if (read_syms() || cur_in_method != current_CS->in_method) {
    destory_win();
  } else {
    if (!syms)
      return;
  }


  if (gwin_sym) {
    if (win_sym_enabled)
      show_win_sym();
    else
      hide_win_sym();

    return;
  }

  gwin_sym = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(gwin_sym), FALSE);
#if WIN32
  set_no_focus(gwin_sym);
#endif

  cur_in_method = current_CS->in_method;

  GtkWidget *hbox_top = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gwin_sym), hbox_top);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_top), GTK_ORIENTATION_VERTICAL);
  gtk_box_pack_start (GTK_BOX (hbox_top), vbox_top, TRUE, TRUE, 0);

  gtk_container_set_border_width (GTK_CONTAINER (vbox_top), 0);

  int i;
  for(i=0; i < symsN; i++) {
    SYM_ROW *psym = &syms[i];
    GtkWidget *hbox_row = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox_row), 0);

    int j;
    for(j=0; j < psym->symN; j++) {
      char *str = psym->sym[j];

      if (!str[0])
         continue;

      GtkWidget *button = gtk_button_new();
      GtkWidget *label = gtk_label_new(str);

      gtk_container_add(GTK_CONTAINER(button), label);
      set_label_font_size(label, hime_font_size_symbol);

      gtk_container_set_border_width (GTK_CONTAINER (button), 0);
      gtk_box_pack_start (GTK_BOX (hbox_row), button, FALSE, FALSE, 0);

      if (utf8_str_N(str) > 0) {
        char phos[512];

        sym_lookup_key(str, phos);

        int phos_len = strlen(phos);

        if (phos_len) {
#if GTK_CHECK_VERSION(2,12,0)
          gtk_widget_set_tooltip_text (button, phos);
#else
          GtkTooltips *button_pho_tips = gtk_tooltips_new ();
          gtk_tooltips_set_tip (GTK_TOOLTIPS (button_pho_tips), button, phos, NULL);
#endif
        }
      }

      g_signal_connect (G_OBJECT (button), "clicked",  G_CALLBACK (cb_button_sym), label);
    }
  }

  gtk_box_pack_start (GTK_BOX (hbox_top), gtk_vseparator_new(), FALSE, FALSE, 0);

  GtkWidget *vbox_arrow = gtk_vbox_new (TRUE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_arrow), GTK_ORIENTATION_VERTICAL);
  gtk_box_pack_start (GTK_BOX (hbox_top), vbox_arrow, TRUE, TRUE, 0);
  GtkWidget *eve_up=gtk_event_box_new(), *eve_down=gtk_event_box_new();
  gtk_event_box_set_visible_window (GTK_EVENT_BOX(eve_up), FALSE);
  gtk_event_box_set_visible_window (GTK_EVENT_BOX(eve_down), FALSE);
  gtk_box_pack_start (GTK_BOX (vbox_arrow), eve_up, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(eve_up), gtk_arrow_new(GTK_ARROW_UP, GTK_SHADOW_IN));
  gtk_box_pack_start (GTK_BOX (vbox_arrow), eve_down, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(eve_down), gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_IN));

  g_signal_connect(G_OBJECT(eve_up),"button-press-event", G_CALLBACK(mouse_button_callback_up_down), (gpointer)1);
  g_signal_connect(G_OBJECT(eve_down),"button-press-event", G_CALLBACK(mouse_button_callback_up_down), NULL);


  gtk_widget_realize (gwin_sym);
#if UNIX
  set_no_focus(gwin_sym);
#else
  win32_init_win(gwin_sym);
#endif

  if (win_sym_enabled)
    gtk_widget_show_all(gwin_sym);

  g_signal_connect (G_OBJECT (gwin_sym), "scroll-event", G_CALLBACK (button_scroll_event), NULL);

#if WIN32
  show_win_sym();
#else
  move_win_sym();
#endif
#if 0
  dbg("in_method:%d\n", current_CS->in_method);
#endif
  return;
}


void toggle_win_sym()
{
  win_sym_enabled^=1;
  create_win_sym();
}

void change_win_sym_font_size()
{
}
