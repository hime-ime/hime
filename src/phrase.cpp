/* Copyright (C) 1994-2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include <stdio.h>
#if UNIX
#include <X11/Xlib.h>
#include <X11/keysym.h>
#endif
#include "hime.h"
#include "gtab.h"
#include "gtab-buf.h"

struct keystruc {
  char *kname;
  KeySym ksym;
  char *str;
  char *str_caps;
};

struct keystruc tran[]={
  {"`", '~'},
  {"0", ')'}, {"1", '!'}, {"2", '@'}, {"3", '#'}, {"4", '$'}, {"5", '%'},
  {"6", '^'}, {"7", '&'}, {"8", '*'}, {"9", '('},
  {"a", 'a'}, {"b", 'b'}, {"c", 'c'}, {"d", 'd'}, {"e", 'e'}, {"f", 'f'},
  {"g", 'g'}, {"h", 'h'}, {"i", 'i'}, {"j", 'j'}, {"k", 'k'}, {"l", 'l'},
  {"m", 'm'}, {"n", 'n'}, {"o", 'o'}, {"p", 'p'}, {"q", 'q'}, {"r", 'r'},
  {"s", 's'}, {"t", 't'}, {"u", 'u'}, {"v", 'v'}, {"w", 'w'}, {"x", 'x'},
  {"y", 'y'}, {"z", 'z'},
  {",", '<'}, {".", '>'}, {";", ':'}, {"'", '"'}, {"/", '?'},
  {"[", '{'}, {"]", '}'}, {"\\", '|'},
  {"-", '_'}, {"=", '+'},
  {"f1",XK_F1},{"f2",XK_F2},{"f3",XK_F3},{"f4",XK_F4},{"f5",XK_F5},{"f6",XK_F6},
  {"f7",XK_F7},{"f8",XK_F8},{"f9",XK_F9},{"f10",XK_F10},{"f11",XK_F11},
  {"f12",XK_F12},
  {"left", XK_Left},  {"right", XK_Right},  {"down", XK_Down},  {"up", XK_Up},
  {"k_ins", XK_KP_Insert}, {"k_del", XK_KP_Delete},  {"k_end", XK_KP_End},
  {"k_down",XK_KP_Down}, {"k_pgup",XK_KP_Prior},
  {"k_up",XK_KP_Up},
  {"k_pgdn",XK_KP_Next},    {"k_left",XK_KP_Left},
  {"k_5",   XK_KP_Begin},  {"k_right", XK_KP_Right}, {"k_home",XK_KP_Home},
  {"k_up",XK_Up}, {"k_pgup",XK_Prior},
  {"kp0", XK_KP_0}, {"kp.", XK_KP_Decimal},
  {"kp1", XK_KP_1}, {"kp2", XK_KP_2}, {"kp3", XK_KP_3},
  {"kp4", XK_KP_4}, {"kp5", XK_KP_5}, {"kp6", XK_KP_6},
  {"kp7", XK_KP_7}, {"kp8", XK_KP_8}, {"kp9", XK_KP_9},
  {"kp/",XK_KP_Divide}, {"kp*", XK_KP_Multiply}, {"kp-", XK_KP_Subtract},
  {"kp+",XK_KP_Add}, {"kpenter",XK_KP_Enter}
};


struct keystruc tran_ctrl[]={
  {",", ','}, {".", '.'}, {";", ';'}, {"'", '\''}, {"/", '/'}, {"?",'?'},
  {"[", '['}, {"]", ']'},
  {":",':'}, {"{",'{'}, {"}",'}'}, {"<",'<'}, {">",'>'}, {"\"",'"'},
};


int tranN=sizeof(tran)/sizeof(tran[0]);
int tran_ctrlN=sizeof(tran_ctrl)/sizeof(tran_ctrl[0]);
extern char *TableDir;

FILE *watch_fopen(char *filename, time_t *pfile_modify_time);
gboolean output_gbuf();
gboolean gtab_cursor_end();
gboolean gtab_phrase_on(), tsin_cursor_end();
void flush_tsin_buffer();
void add_to_tsin_buf_str(char *str);

static time_t file_modify_time;
static time_t ctrl_file_modify_time;

void load_phrase(char *fname, time_t *modtime, struct keystruc *tr, int trN)
{
  FILE *fp;
  char kname[32];
  char ttt[512];

//  dbg("load_phrase %s\n", fname);
  if ((fp=watch_fopen(fname, modtime)) == NULL) {
    return;
  }

//  dbg("load succcc %s\n", fname);
  skip_utf8_sigature(fp);

  while (!feof(fp)) {
    int i,j;
    char str[512];

    kname[0]=str[0]=0;
    myfgets(ttt, sizeof(ttt), fp);
    if (ttt[0]=='#')
      continue;
    for(i=0; ttt[i]!=' ' && ttt[i]!=9 && ttt[i]; i++)
      kname[i]=ttt[i];

    kname[i]=0;
    gboolean is_upper = FALSE;

    if (!(kname[0]&0x80) && isupper(kname[0])) {
       is_upper = TRUE;
       kname[0] = tolower(kname[0]);
    }

    while((ttt[i]==' ' || ttt[i]==9) && ttt[i])
      i++;

    for(j=0; ttt[i]!='\n' && ttt[i]; i++,j++)
      str[j]=ttt[i];

    if (!str[0] || !kname[0])
      continue;

    str[j]=0;


    for(i=0; i < trN; i++)
      if (!strcmp(kname, tr[i].kname))
            break;
    if (i==trN) {
      dbg("unknown key: %s\n", kname);
      continue;
    }

    if (is_upper)
      tr[i].str_caps = strdup(str);
    else
      tr[i].str = strdup(str);
  }

}


void free_phrase()
{
  int i;

  for(i=0; i < tranN; i++)
    free(tran[i].str);
}


gboolean feed_phrase(KeySym ksym, int state)
{
  int i;

//  dbg("ksym:%x %c\n", ksym, ksym);
  load_phrase("phrase.table", &file_modify_time, tran, tranN);
  load_phrase("phrase-ctrl.table", &ctrl_file_modify_time, tran_ctrl, tran_ctrlN);


  if (ksym < 0x7f && isupper(ksym))
    ksym = tolower(ksym);

  struct keystruc *tr;
  int trN;

  if (state & ControlMask) {
    tr = tran_ctrl;
    trN = tran_ctrlN;
  } else {
    tr = tran;
    trN = tranN;
  }

  char *str;

  for(i=0; i < trN; i++) {
    if (tr[i].ksym!= ksym)
      continue;

    str = ((state & LockMask) && tr[i].str_caps) ? tr[i].str_caps : tr[i].str;

    if (str) {
//send_it:
#if USE_TSIN
      if (current_method_type() == method_type_TSIN && current_CS->im_state == HIME_STATE_CHINESE) {
        add_to_tsin_buf_str(str);
        if (tsin_cursor_end())
          flush_tsin_buffer();
      }
      else
#endif
      if (gtab_phrase_on()) {
        insert_gbuf_nokey(str);
        if (gtab_cursor_end())
          output_gbuf();
      } else
        send_text(str);
      return TRUE;
    }
  }

#if 0
  char tt[2];
  if ((state&(ControlMask|ShiftMask|Mod1Mask|Mod4Mask|Mod5Mask))==ShiftMask && ksym>=' ' && ksym < 0x7e) {
    str = tt;
    tt[0]=ksym;
    tt[1]=0;
    goto send_it;
  }
#endif

  return FALSE;
}
