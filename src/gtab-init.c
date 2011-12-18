/* Copyright (C) 2004-2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
#include <regex.h>
#include "hime.h"
#include "gtab.h"
#include "pho.h"
#include "hime-conf.h"
#include "hime-endian.h"
#include "gtab-buf.h"
#include "tsin.h"
#include "gst.h"

GTAB_space_pressed_E _gtab_space_auto_first;
char **seltab;
extern gboolean test_mode;
extern unich_t *fullchar[];
INMD *cur_inmd;
#if UNIX
GTAB_ST ggg = {.sel1st_i=MAX_SELKEY - 1};
#else
GTAB_ST ggg;
#endif

/* for array30-like quick code */
static char keyrow[]=
	      "qwertyuiop"
	      "asdfghjkl;"
	      "zxcvbnm,./";

gboolean gtab_phrase_on()
{
  int val = cur_inmd && cur_inmd->DefChars >500 &&
(gtab_auto_select_by_phrase==GTAB_AUTO_SELECT_BY_PHRASE_YES||
(gtab_auto_select_by_phrase==GTAB_AUTO_SELECT_BY_PHRASE_AUTO&&(cur_inmd->flag&FLAG_AUTO_SELECT_BY_PHRASE)));

return val;
}

int key_col(char cha)
{
  char *p = strchr(keyrow, cha);
  if (!p)
    return 0;
  return (p - keyrow)%10;
}

void init_seltab(char ***p)
{
  if (!*p) {
    *p = tmalloc(char *, MAX_SELKEY);
    int i;
    for(i=0; i < MAX_SELKEY; i++)
      (*p)[i]=(char *)zmalloc(MAX_CIN_PHR);
  }
}

time_t file_mtime(char *fname)
{
  struct stat st;

  if (stat(fname, &st) < 0)
    return 0;

  return st.st_mtime;
}


time_t find_tab_file(char *fname, char *out_file)
{
  get_hime_user_fname(fname, out_file);
  time_t mtime = file_mtime(out_file);

  if (!mtime) {
    strcat(strcpy(out_file,TableDir),"/");
    strcat(out_file, fname);
    if (!(mtime = file_mtime(out_file))) {
      dbg("init_tab:1 err open %s\n", out_file);
      return 0;
    }
  }

  return mtime;
}

void init_gtab(int inmdno)
{
  FILE *fp;
  char ttt[128],uuu[128];
  int i;
  INMD *inp=&inmd[inmdno];
  struct TableHead th;

  ggg.sel1st_i=MAX_SELKEY - 1;
  init_seltab(&seltab);

//  current_CS->b_half_full_char = FALSE;
  if (!inmd[inmdno].filename || !strcmp(inmd[inmdno].filename,"-")) {
//    dbg("filename is empty\n");
    return;
  }

  time_t mtime;

  if (!(mtime = find_tab_file(inmd[inmdno].filename, ttt)))
    return;


  char append[64], append_user[128];
  strcat(strcpy(append, inmd[inmdno].filename), ".append");
  get_hime_user_fname(append, append_user);
  time_t mtime_append = file_mtime(append_user);

  if (mtime_append) {
    char append_user_gtab[128];

    strcat(strcpy(append_user_gtab, append_user), ".gtab");
    time_t mtime_append_gtab = file_mtime(append_user_gtab);

    if (mtime_append_gtab < mtime || mtime_append_gtab < mtime_append) {
      char exe[256];

#if WIN32
      sprintf(exe, "\"%s\" \"%s\" \"%s\"", ttt, append_user, append_user_gtab);
      dbg("exe %s\n", exe);
      win32exec_para("hime-gtab-merge", exe);
      Sleep(1000);
#else
      sprintf(exe, HIME_BIN_DIR"/hime-gtab-merge %s %s %s", ttt, append_user, append_user_gtab);
      dbg("exe %s\n", exe);
      system(exe);
#endif

      mtime_append_gtab = file_mtime(append_user_gtab);
    }

    if (mtime_append_gtab) {
      strcpy(ttt, append_user_gtab);
      mtime = mtime_append_gtab;
      free(inmd[inmdno].filename_append);
      inmd[inmdno].filename_append = strdup(append_user_gtab);
    }
  }

  if (mtime == inp->file_modify_time) {
//    dbg("unchanged\n");
//    set_gtab_input_method_name(inp->cname);
    cur_inmd=inp;

    if (gtab_space_auto_first == GTAB_space_auto_first_none)
      _gtab_space_auto_first = cur_inmd->space_style;
    else
      _gtab_space_auto_first = (GTAB_space_pressed_E)gtab_space_auto_first;

    if (gtab_phrase_on() && _gtab_space_auto_first == GTAB_space_auto_first_any)
      _gtab_space_auto_first = GTAB_space_auto_first_nofull;

    return;    /* table is already loaded */
  }

  inp->file_modify_time = mtime;

  if ((fp=fopen(ttt, "rb"))==NULL)
    p_err("init_tab:2 err open %s", ttt);

  dbg("gtab file %s\n", ttt);

  strcpy(uuu,ttt);

  fread(&th,1,sizeof(th),fp);

  if (th.keybits<6 || th.keybits>7)
    th.keybits = 6;

  inp->keybits = th.keybits;
  dbg("keybits:%d\n", th.keybits);

#if NEED_SWAP
  swap_byte_4(&th.version);
  swap_byte_4(&th.flag);
  swap_byte_4(&th.space_style);
  swap_byte_4(&th.KeyS);
  swap_byte_4(&th.MaxPress);
  swap_byte_4(&th.M_DUP_SEL);
  swap_byte_4(&th.DefC);
#endif

  if (th.MaxPress*th.keybits > 32) {
    inp->max_keyN = 64 / th.keybits;
    inp->key64 = TRUE;
    dbg("it's a 64-bit .gtab\n");
  } else {
    inp->max_keyN = 32 / th.keybits;
  }

  free(inp->endkey);
  inp->endkey = strdup(th.endkey);

  if (th.flag & FLAG_GTAB_SYM_KBM)
    dbg("symbol kbm\n");

  if (th.flag & FLAG_PHRASE_AUTO_SKIP_ENDKEY)
    dbg("PHRASE_AUTO_SKIP_ENDKEY\n");

  fread(ttt, 1, th.KeyS, fp);
  dbg("KeyS %d\n", th.KeyS);

  if (inp->keyname)
    free(inp->keyname);
  inp->keyname = tmalloc(char, (th.KeyS + 3) * CH_SZ);
  fread(inp->keyname, CH_SZ, th.KeyS, fp);
  inp->WILD_QUES=th.KeyS+1;
  inp->WILD_STAR=th.KeyS+2;
#if 0
  utf8cpy(&inp->keyname[inp->WILD_QUES*CH_SZ], _(_L("？")));  /* for wild card */
  utf8cpy(&inp->keyname[inp->WILD_STAR*CH_SZ], _(_L("＊")));
#else
  utf8cpy(&inp->keyname[inp->WILD_QUES*CH_SZ], "?");  /* for wild card */
  utf8cpy(&inp->keyname[inp->WILD_STAR*CH_SZ], "*");
#endif

  // for boshiamy
  gboolean all_full_ascii = TRUE;
  char keyname_lookup[256];

  bzero(keyname_lookup, sizeof(keyname_lookup));
  for(i=1; i < th.KeyS; i++) {
    char *keyname = &inp->keyname[i*CH_SZ];
    int len = utf8_sz(keyname);
    int j;

    if (len==1 && utf8_sz(keyname + 1)) { // array30
      all_full_ascii = FALSE;
      break;
    }

#define FULLN (127 - ' ')

    for(j=0; j < FULLN; j++)
      if (!memcmp(_(fullchar[j]), keyname, len)) {
        break;
      }

    if (j==FULLN) {
      dbg("all_full_ascii %d\n", j);
      all_full_ascii = FALSE;
      break;
    }

    keyname_lookup[i] = ' ' + j;
  }


  if (all_full_ascii) {
    dbg("all_full_ascii\n");
    int mkeys = 1<< th.keybits;
    free(inp->keyname_lookup);
    inp->keyname_lookup = (char *)malloc(sizeof(char) * mkeys);
    memcpy(inp->keyname_lookup, keyname_lookup, mkeys);
  }

  inp->KeyS=th.KeyS;
  inp->MaxPress=th.MaxPress;
  inp->DefChars=th.DefC;
  free(inp->selkey);

  if (th.selkey[sizeof(th.selkey)-1]) {
    char tt[32];
    bzero(tt, sizeof(tt));
    memcpy(tt,th.selkey, sizeof(th.selkey));
    strcat(tt, th.selkey2);
    inp->selkey = strdup(tt);
  } else
    inp->selkey = strdup(th.selkey);

  dbg("selkey %s\n", inp->selkey);

  inp->M_DUP_SEL=th.M_DUP_SEL;
  inp->space_style=th.space_style;
  inp->flag=th.flag;
  free(inp->cname);
  inp->cname = strdup(th.cname);

//  dbg("MaxPress:%d  M_DUP_SEL:%d\n", th.MaxPress, th.M_DUP_SEL);

  free(inp->keymap);
  inp->keymap = tzmalloc(char, 128);

  if (!(th.flag & FLAG_GTAB_SYM_KBM)) {
    inp->keymap[(int)'?']=inp->WILD_QUES;
    if (!strchr(th.selkey, '*'))
      inp->keymap[(int)'*']=inp->WILD_STAR;
  }

  free(inp->keycol);
  inp->keycol=tzmalloc(char, th.KeyS+1);
  for(i=0;i<th.KeyS;i++) {
    dbg("%c", ttt[i]);
    inp->keymap[(int)ttt[i]]=i;
//    dbg("%d %d %c\n", i, inp->keymap[(int)ttt[i]], ttt[i]);
    if (!BITON(inp->flag, FLAG_KEEP_KEY_CASE))
      inp->keymap[toupper(ttt[i])]=i;
    inp->keycol[i]=key_col(ttt[i]);
  }
  dbg("\n");

  free(inp->idx1);
  inp->idx1 = tmalloc(gtab_idx1_t, th.KeyS+1);
  fread(inp->idx1, sizeof(gtab_idx1_t), th.KeyS+1, fp);
#if NEED_SWAP
  for(i=0; i <= th.KeyS+1; i++)
    swap_byte_4(&inp->idx1[i]);
#endif
  /* printf("chars: %d\n",th.DefC); */
  dbg("inmdno: %d th.KeyS:%d  MaxPress:%d\n", inmdno, th.KeyS, th.MaxPress);

  if (inp->key64) {
    if (inp->tbl64) {
      dbg("free %x\n", inp->tbl64);
      free(inp->tbl64);
    }

    if ((inp->tbl64=tmalloc(ITEM64, th.DefC))==NULL) {
      p_err("malloc err");
    }

    fread(inp->tbl64, sizeof(ITEM64), th.DefC, fp);
#if NEED_SWAP
    for(i=0; i < th.DefC; i++) {
      swap_byte_8(&inp->tbl64[i].key);
    }
#endif
  } else {
    if (inp->tbl) {
      dbg("free %x\n", inp->tbl);
      free(inp->tbl);
    }

    if ((inp->tbl=tmalloc(ITEM, th.DefC))==NULL) {
      p_err("malloc err");
    }

    fread(inp->tbl,sizeof(ITEM),th.DefC, fp);
#if NEED_SWAP
    for(i=0; i < th.DefC; i++) {
      swap_byte_4(&inp->tbl[i].key);
    }
#endif
  }

  dbg("chars %d\n", th.DefC);

  free(inp->qkeys);
  inp->use_quick= th.qkeys.quick1[1][0][0] != 0;  // only array 30 use this
  if (inp->use_quick) {
    inp->qkeys = tmalloc(QUICK_KEYS, 1);
    memcpy(inp->qkeys, &th.qkeys, sizeof(th.qkeys));
  }

  fread(&inp->phrnum, sizeof(int), 1, fp);
#if NEED_SWAP
    swap_byte_4(&inp->phrnum);
    for(i=0; i < inp->phrnum; i++) {
      swap_byte_4(&inp->phrnum);
    }
#endif
  dbg("inp->phrnum: %d\n", inp->phrnum);
  free(inp->phridx);
  inp->phridx = tmalloc(int, inp->phrnum);
  fread(inp->phridx, sizeof(int), inp->phrnum, fp);
#if NEED_SWAP
    for(i=0; i < inp->phrnum; i++) {
      swap_byte_4(&inp->phridx[i]);
    }
#endif

#if 0
  for(i=0; i < inp->phrnum; i++)
    dbg("inp->phridx %d %d\n", i, inp->phridx[i]);
#endif

  int nbuf = 0;
  if (inp->phrnum)
    nbuf = inp->phridx[inp->phrnum-1];

  free(inp->phrbuf);
  inp->phrbuf = (char *)malloc(nbuf);
  fread(inp->phrbuf, 1, nbuf, fp);

  fclose(fp);

  cur_inmd=inp;
//    reset_inp();
//  set_gtab_input_method_name(inp->cname);
//  DispInArea();

  dbg("key64: %d\n", inp->key64);

  if (gtab_space_auto_first == GTAB_space_auto_first_none)
    _gtab_space_auto_first = th.space_style;
  else
    _gtab_space_auto_first = (GTAB_space_pressed_E) gtab_space_auto_first;

  if (gtab_phrase_on() && _gtab_space_auto_first == GTAB_space_auto_first_any)
    _gtab_space_auto_first = GTAB_space_auto_first_nofull;

  inp->last_k_bitn = (((cur_inmd->key64 ? 64:32) / inp->keybits) - 1) * inp->keybits;
  inp->kmask = (1 << th.keybits) - 1;

#if 0
  for(i='A'; i < 127; i++)
    printf("%d] %c %d\n", i, i, inp->keymap[i]);
#endif
#if 0
  for(i=0; i < Min(100,th.DefC) ; i++) {
    u_char *ch = tblch(i);
    dbg("%d] %x %c%c%c\n", i, *((int *)inp->tbl[i].key), ch[0], ch[1], ch[2]);
  }
#endif
}
