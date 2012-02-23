/* Copyright (C) 1994-2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#define InAreaX (0)

#include "hime.h"
#include "intcode.h"
#include "pho.h"
#include "gst.h"
#include "im-client/hime-im-client-attr.h"
#include "hime-module.h"
#include "hime-module-cb.h"

extern GtkWidget *gwin_int;
HIME_module_main_functions gmf;
int current_intcode = INTCODE_UTF32;

static char inch[MAX_INTCODE];
int intcode_cin;

void create_win_intcode();
void module_show_win();

int module_init_win(HIME_module_main_functions *funcs)
{
  intcode_cin=0;
  gmf = *funcs;
  create_win_intcode();
  return TRUE;
}

static int h2i(int x)
{
  return (x<='9'?x-'0':x-'A'+10);
}

static void utf32to8(char *t, char *s)
{
  gsize rn,wn=0;
  GError *err = NULL;
  char *utf8 = g_convert(s, 4, "UTF-8", "UTF-32", &rn, &wn, &err);

  if (utf8) {
    memcpy(t, utf8, wn);
    g_free(utf8);
  }

  t[wn]=0;
}


void big5_utf8_n(char *s, int len, char out[])
{
  out[0]=0;

  GError *err = NULL;
  gsize rn, wn;
  char *utf8 = g_convert(s, len, "UTF-8", "Big5", &rn, &wn, &err);

  if (err) {
    dbg("big5_utf8  convert error\n");
    out[0]=0;
//    abort();
    return;
  }

  strcpy(out, utf8);
  g_free(utf8);
}


void big5_utf8(char *s, char out[])
{
  big5_utf8_n(s, strlen(s), out);
}


static unich_t *dstr[] = { "０", "１", "２", "３", "４", "５", "６", "７", "８", "９", "Ａ", "Ｂ", "Ｃ", "Ｄ", "Ｅ", "Ｆ" };

void disp_int(int index, char *intcode);
void clear_int_code_all();

gboolean module_feedkey(int key, int kvstate)
{
  int i;
#if 0
  if (key <= XK_KP_9 && key >= XK_KP_0)
    key -= XK_KP_0 - '0';
#endif
  key=toupper(key);
  if (key==XK_BackSpace||key==XK_Delete) {
    if (intcode_cin)
      intcode_cin--;
    else
      return 0;

    goto dispIn;
  }
  else
  if ((key<'0'||key>'F'||(key>'9' && key<'A')) && (key!=' ')){
    return 0;
  }

  if (current_intcode==INTCODE_BIG5) {
    if (intcode_cin==0 && key<'8')
      return 1;
    if (intcode_cin==1 && inch[0]=='F' && key=='F')
      return 1;
    if (intcode_cin==2 && (key<'4' || (key>'7' && key<'A')))
      return 1;
    if (intcode_cin==3 && (inch[2]=='7'||inch[2]=='F') && key=='F')
      return 1;
  }

  if (!intcode_cin && key==' ')
    return 0;
  if ((intcode_cin<MAX_INTCODE-1 || (current_intcode!=INTCODE_BIG5 && intcode_cin < MAX_INTCODE)) && key!=' ')
    inch[intcode_cin++]=key;

dispIn:
  clear_int_code_all();

#if 1
  if (intcode_cin)
    module_show_win();
#endif

  for(i=0;i<intcode_cin;i++) {
    disp_int(i, _(dstr[h2i(inch[i])]));
  }

  if (((current_intcode==INTCODE_BIG5 && intcode_cin==4) ||
       (current_intcode==INTCODE_UTF32 && intcode_cin==6)) || key==' ') {
    u_char utf8[CH_SZ+1];

    if (current_intcode==INTCODE_BIG5) {
      u_char ttt[4];
      ttt[2]=ttt[3]=0;
      ttt[0]=(h2i(inch[0])<<4)+h2i(inch[1]);
      ttt[1]=(h2i(inch[2])<<4)+h2i(inch[3]);
      big5_utf8((char *)ttt, (char *)utf8);
    } else {
      int i;
      u_int v = 0;

      for(i=0; i < intcode_cin; i++) {
        v <<= 4;
        v |= h2i(inch[i]);
      }

      utf32to8((char *)utf8, (char *)&v);
    }

    gmf.mf_send_utf8_ch((char *)utf8);
    intcode_cin=0;

    clear_int_code_all();
  }

  return 1;
}

extern GtkWidget *gwin_int;

int module_get_preedit(char *str, HIME_PREEDIT_ATTR attr[], int *cursor, int *comp_flag)
{
  *comp_flag = intcode_cin>0;
#if 1
  if (gwin_int && GTK_WIDGET_VISIBLE(gwin_int))
    *comp_flag|=2;
#endif
//  dbg("comp_len %x\n", *sub_comp_len);
  str[0]=0;
  *cursor=0;
  return 0;
}

int module_feedkey_release(KeySym xkey, int kbstate)
{
	return 0;
}

int module_flush_input()
{
  return FALSE;
}

int module_reset()
{
  return TRUE;
}
