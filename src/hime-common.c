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

PIN_JUYIN *pin_juyin;

int text_pho_N=3;

gboolean b_use_full_space = TRUE;

static char text_pho[6][CH_SZ];

void bell()
{
  if (hime_bell_off)
    return;

#if 1
  XBell(dpy, hime_bell_volume);
#else
  gdk_beep();
#endif
//  abort();
}

void case_inverse(KeySym *xkey, int shift_m)
{
  if (*xkey > 0x7e)
    return;

  if (shift_m) {
    if (islower(*xkey))
      *xkey-=0x20;
  } else
  if (isupper(*xkey))
    *xkey+=0x20;
}

gint64 current_time()
{
  struct timeval tval;

  gettimeofday(&tval, NULL);
  return (gint64)tval.tv_sec * 1000000 + tval.tv_usec;
}

void disp_pho_sub(GtkWidget *label, int index, char *pho)
{
  if (!label)
    return;

  if (index>=text_pho_N)
    return;

  if (pho[0]==' ' && !pin_juyin) {
    u8cpy(text_pho[index], "　"); /* Full width space */
  }
  else {
    u8cpy(text_pho[index], pho);
  }
  char s[text_pho_N * CH_SZ+1];

  int tn = 0;
  int i;
  for(i=0; i < text_pho_N; i++) {
    int n = utf8cpy(s + tn, text_pho[i]);
    tn += n;
  }

//  gtk_widget_show(label);
  gtk_label_set_text(GTK_LABEL(label), s);
}

void exec_hime_setup()
{
  dbg("exec hime\n");
  if (geteuid() < 100 || getegid() < 100)
    return;

  system(HIME_BIN_DIR"/hime-setup &");
}

void set_label_font_size(GtkWidget *label, int size)
{
  if (! GTK_IS_WIDGET(label))
    return;

  PangoContext *pango_context = gtk_widget_get_pango_context (label);
  PangoFontDescription* font=pango_context_get_font_description (pango_context);
#if 0
  pango_font_description_set_family(font, hime_font_name);
  pango_font_description_set_size(font, PANGO_SCALE * size);
#else
  char tt[256];
  sprintf(tt, "%s %d", hime_font_name, size);
  PangoFontDescription* nfont = pango_font_description_from_string(tt);
  pango_font_description_merge(font, nfont, TRUE);
  pango_font_description_free(nfont);
#endif
  gtk_widget_override_font(label, font);
}

// the width of ascii space in firefly song
void set_label_space(GtkWidget *label)
{
  gtk_label_set_text(GTK_LABEL(label), "\xe3\x80\x80");
  return;
}

void set_no_focus(GtkWidget *win)
{
  gdk_window_set_override_redirect(gtk_widget_get_window(win), TRUE);
  gtk_window_set_accept_focus(GTK_WINDOW(win), FALSE);
  gtk_window_set_focus_on_map (GTK_WINDOW(win), FALSE);
  gtk_window_set_resizable (GTK_WINDOW (win), FALSE);
}

#if !USE_TSIN
void change_tsin_color(){}
void change_tsin_font_size(){}
void change_win0_style(){}
void destroy_win0(){}
void destroy_win1(){}
void free_tsin(){}
void load_tsin_db(){}
void tsin_reset_in_pho(){}
void tsin_reset_in_pho0(){}
void tsin_toggle_half_full(){}
#endif
