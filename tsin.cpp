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

#include <string.h>

#include "hime.h"
#include "pho.h"
#include "tsin.h"
#include "hime-conf.h"
#include "tsin-parse.h"
#include "win-save-phrase.h"
#include "gst.h"
#include "gtab.h"
#include "pho-status.h"

extern int ph_key_sz;
extern GtkWidget *gwin1;
gint64 key_press_time, key_press_time_ctrl;
extern gboolean b_hsu_kbm;
extern gboolean test_mode;

extern char *pho_chars[];

TSIN_ST tss;

gboolean typ_pho_empty();
void mask_tone(phokey_t *pho, int plen, char *tone_off);

extern u_short hash_pho[];
extern PHOKBM phkbm;

extern int hashidx[TSIN_HASH_N];
// gboolean eng_ph=TRUE;  // english(FALSE) <-> pho(juyin, TRUE)

void clrin_pho(), hide_win0();
void show_tsin_stat();
void save_CS_current_to_temp();

gboolean tsin_pho_mode()
{
  return current_CS && current_CS->tsin_pho_mode;
}

void set_tsin_pho_mode0(ClientState *cs)
{
  if (!cs)
    return;
  cs->tsin_pho_mode = 1;
  save_CS_current_to_temp();
}

void set_tsin_pho_mode()
{
  set_tsin_pho_mode0(current_CS);
  show_tsin_stat();
}

gboolean tsin_cursor_end()
{
  return tss.c_idx==tss.c_len;
}

gboolean tsin_has_input();
static void clrin_pho_tsin()
{
  clrin_pho();

  if (!tsin_has_input() && hime_pop_up_win)
    hide_win0();
}

gboolean pho_has_input();
gboolean hime_edit_display_ap_only();

gboolean tsin_has_input()
{
  gboolean v = (!hime_edit_display_ap_only() && tss.c_len) || pho_has_input();
//  dbg("tsin_has_input %d\n", v);
  return v;
}


void disp_char(int index, char *ch);

static void disp_char_chbuf(int idx)
{
//  dbg("disp_char_chbuf %d '%s' '%s'\n", idx, tss.chpho[idx].ch, tss.chpho[idx].cha);
  disp_char(idx, tss.chpho[idx].ch);
}

static void init_chpho_i(int i)
{
//  dbg("init_chpho_i %d\n", i);
  tss.chpho[i].ch = tss.chpho[i].cha;
  tss.chpho[i].ch[0]=' ';
  tss.chpho[i].ch[1]=0;
  tss.chpho[i].flag=0;
  tss.chpho[i].psta=-1;
}

void clr_tsin_cursor(int index);

static void clrcursor()
{
  clr_tsin_cursor(tss.c_idx);
}

void set_cursor_tsin(int index);

void drawcursor()
{
  clr_tsin_cursor(tss.last_cursor_idx);
  tss.last_cursor_idx = tss.c_idx;

  if (!tss.c_len)
    return;

  if (tss.c_idx == tss.c_len) {
    if (!tsin_pho_mode()) {
      if (tss.tsin_half_full) {
        disp_char(tss.c_idx,"  ");
        set_cursor_tsin(tss.c_idx);
      } else {
        disp_char(tss.c_idx, " ");
        set_cursor_tsin(tss.c_idx);
      }
    }
  }
  else {
    set_cursor_tsin(tss.c_idx);
  }
}

void chpho_extract(CHPHO *chph, int len, phokey_t *pho, char *ch)
{
   int i;
   int ofs=0;
   ch[0]=0;

   for(i=0; i < len; i++) {
      if (pho)
        pho[i] = chph[i].pho;

      char *str = chph[i].ch;
      strcat(ch + ofs, str);
      ofs+=strlen(str);
   }
//   dbg("chpho_extract %s\n", ch);
}

// in tsin db, # of phokey = # of character, use this to extract only the first characer
static void chpho_extract_cha(CHPHO *chph, int len, phokey_t *pho, char *ch)
{
   int i;
   int ofs=0;

   for(i=0; i < len; i++) {
      if (pho)
        pho[i] = chph[i].pho;
      ofs += u8cpy(ch + ofs, chph[i].ch);
   }

   ch[ofs]=0;
//   dbg("chpho_extract %s\n", ch);
}

void chpho_get_str(int idx, int len, char *ch)
{
  int ofs=0, i;
  for(i=0; i < len; i++) {
	int u8len = u8cpy(&ch[ofs], tss.chpho[idx+i].ch);
    ofs+=u8len;
  }

  ch[ofs]=0;
}


void inc_pho_count(phokey_t key, int ch_idx);
int ch_key_to_ch_pho_idx(phokey_t phkey, char *big5);
void inc_dec_tsin_use_count(void *pho, char *ch, int prlen);
void lookup_gtabn(char *ch, char *);

static void putbuf(int len)
{
  u_char tt[CH_SZ * (MAX_PH_BF_EXT+1) + 1];
  int i,idx;
#if WIN32
  if (test_mode)
    return;
#endif

//  dbg("putbuf:%d\n", len);
#if 1
  // update phrase reference count
  if (len >= 2) {
    for(i=0; i < len; i++) {
//      dbg("flag %d %x\n", i, tss.chpho[i].flag);
      if (!BITON(tss.chpho[i].flag, FLAG_CHPHO_PHRASE_HEAD)) {
        continue;
      }

      int j;
      for(j=i+1; j < len; j++)
        if (tss.chpho[j].psta != i)
          break;

      int phrlen = j - i;
      if (phrlen < 1)
        continue;

      phokey_t pho[MAX_PHRASE_LEN];
      char ch[MAX_PHRASE_LEN * CH_SZ * 2];

      chpho_extract(&tss.chpho[i], phrlen, pho, ch);

      inc_dec_tsin_use_count(pho, ch, phrlen);
    }
  }
#endif

  for(idx=i=0;i<len;i++) {
#if 0
    int len = utf8_sz(tss.chpho[i].ch);
#else
    int len = strlen(tss.chpho[i].ch);
#endif

    if (tss.chpho[i].pho && len > 1) {
      int pho_idx = ch_key_to_ch_pho_idx(tss.chpho[i].pho, tss.chpho[i].ch);
      if (pho_idx >= 0)
        inc_pho_count(tss.chpho[i].pho, pho_idx);
    }

    memcpy(&tt[idx], tss.chpho[i].ch, len);
    idx += len;
  }

  tt[idx]=0;
  send_text((char *)tt);
  lookup_gtabn((char *)tt, NULL);
}


void hide_char(int index);

static void prbuf()
{
  int i;

//  dbg("prbuf\n");
  for(i=0;i<tss.c_len;i++)
	if (!(tss.chpho[i].flag & FLAG_CHPHO_PHO_PHRASE))
		tss.chpho[i].ch=tss.chpho[i].cha;

  for(i=0; i < tss.c_len; i++)
    disp_char_chbuf(i);

  for(i=tss.c_len; i < MAX_PH_BF_EXT; i++) {
    hide_char(i);
  }

  drawcursor();
}


void disp_tsin_pho(int index, char *pho);

static void disp_in_area_pho_tsin()
{
  int i;

  if (pin_juyin) {
    for(i=0;i<6;i++) {
      disp_tsin_pho(i, &poo.inph[i]);
    }
  } else {
    for(i=0;i<4;i++) {
      if (poo.typ_pho[0]==BACK_QUOTE_NO && i==1)
        break;
      disp_tsin_pho(i, &pho_chars[i][poo.typ_pho[i] * PHO_CHAR_LEN]);
    }
  }
}

static int get_in_area_pho_tsin_str(char *out)
{
  int i;
  int outN=0;

  if (pin_juyin) {
    for(i=0;i<6;i++)
      if (poo.inph[i])
        out[outN++] = poo.inph[i];
  } else {
    for(i=0;i<4;i++)
      if (poo.typ_pho[i]) {
        outN+=u8cpy(out+outN, &pho_chars[i][poo.typ_pho[i] * PHO_CHAR_LEN]);
      }
  }

  out[outN]=0;
  return outN;
}

void clear_chars_all();

static void clear_match()
{
  tss.ph_sta=-1;
}

static void clr_ch_buf()
{
  int i;
  for(i=0; i < MAX_PH_BF_EXT; i++) {
    init_chpho_i(i);
  }

  clear_match();
}


static void clear_ch_buf_sel_area()
{
  clear_chars_all();
  tss.c_len=tss.c_idx=0; tss.ph_sta=-1;
  tss.full_match = FALSE;
  clr_ch_buf();
  drawcursor();
}

static void close_selection_win();

static void clear_tsin_buffer()
{
  clear_ch_buf_sel_area();
  close_selection_win();
  tss.pre_selN = 0;
  tss.tsin_buffer_editing = 0; //buffer editing is finished
}

void clr_in_area_pho_tsin();
void close_win_pho_near();
void compact_win0();

void tsin_reset_in_pho0()
{
//  prbuf();
  clr_in_area_pho_tsin();
  close_selection_win();
  tss.pre_selN = 0;
//  tss.pho_menu_idx = 0;
  drawcursor();
  close_win_pho_near();
}


void tsin_reset_in_pho()
{
  clrin_pho_tsin();
  tsin_reset_in_pho0();
}


void flush_tsin_buffer()
{
#if WIN32
  if (test_mode)
    return;
#endif
  tsin_reset_in_pho();

  if (hime_pop_up_win)
    hide_win0();

  if (tss.c_len) {
    putbuf(tss.c_len);
    compact_win0();
    clear_ch_buf_sel_area();
    clear_tsin_buffer();
    return;
  }

  return;
}


void disp_tsin_eng_pho(int eng_pho), disp_tray_icon();

void show_tsin_stat()
{
#if TRAY_ENABLED
  disp_tray_icon();
#endif
  disp_tsin_eng_pho(tsin_pho_mode());
}

void load_tsin_db();

#if 0
void nputs(u_char *s, u_char len)
{
  char tt[16];

  memcpy(tt, s, len*CH_SZ);
  tt[len*CH_SZ]=0;
  dbg("%s", tt);
}


static void dump_tsidx(int i)
{
  phokey_t pho[MAX_PHRASE_LEN];
  u_char ch[MAX_PHRASE_LEN*CH_SZ];
  usecount_t usecount;
  u_char len;

  load_tsin_entry(i, &len, &usecount, pho, ch);

  int j;
  for(j=0; j < len; j++) {
    prph(pho[j]);
    dbg(" ");
  }

  nputs(ch, len);
  dbg("\n");
}


static void dump_tsidx_all()
{
  int i;

  for(i=0; i < phcount; i++) {
    dump_tsidx(i);
  }

  dbg("************************************************\n");
  for(i=0; i < 254; i++) {
    dbg("%d]%d ", i, hashidx[i]);
    dump_tsidx(hashidx[i]);
  }
}

#endif

void load_tab_pho_file();
void show_win0();

void init_pre_sel()
{
  if (!tss.pre_sel)
    tss.pre_sel=tzmalloc(PRE_SEL, 10);
}

void init_tab_pp(gboolean init)
{
  if (!tss.chpho)
    tss.chpho=tzmalloc(CHPHO, MAX_PH_BF_EXT);

//  tss.ph_sta_last = -1;

  init_pre_sel();

  if (!ch_pho)
    load_tab_pho_file();

  load_tsin_db();

  if (init)
    clr_ch_buf();

  show_tsin_stat();
  if (init)
    clear_ch_buf_sel_area();

  if (!hime_pop_up_win)
    show_win0();
}


static void move_cursor_end()
{
  clrcursor();
  tss.c_idx=tss.c_len;
  drawcursor();
}

gboolean save_phrase_to_db2(CHPHO *chph, int len);

void save_phrase(int save_frm, int len)
{
#if WIN32
  if (test_mode)
    return;
#endif

  int save_to = save_frm + len -1;
  if (len <= 0 || len > MAX_PHRASE_LEN)
    return;

  int i;
  for(i=save_frm;i<=save_to;i++) {
    if (tss.chpho[i].pho)
      continue;
    phokey_t tpho[32];
    tpho[0]=0;

    utf8_pho_keys(tss.chpho[i].ch, tpho);

    if (!tpho[0])
      return;

    tss.chpho[i].pho = tpho[0];
  }

  if (!save_phrase_to_db2(&tss.chpho[save_frm], len)) {
    bell();
  }

  tss.ph_sta=-1;
  move_cursor_end();
  return;
}


static void set_fixed(int idx, int len)
{
  int i;
  for(i=idx; i < idx+len; i++) {
    tss.chpho[i].flag |= FLAG_CHPHO_FIXED;
    tss.chpho[i].flag &= ~FLAG_CHPHO_PHRASE_USER_HEAD;
  }
}

#define PH_SHIFT_N (tsin_buffer_size - 1)

static void shift_ins()
{
   int j;
//   dbg("shift_ins()\n");

   if (!tss.c_idx && tss.c_len >= PH_SHIFT_N) {
     tss.c_len--;
   }
   else
   if (tss.c_len >= PH_SHIFT_N) {
     int ofs;

     // set it fixed so that it will not cause partial phrase in the beginning
     int fixedlen = tss.c_len - 10;
     if (fixedlen <= 0)
       fixedlen = 1;
     set_fixed(0, fixedlen);

     ofs = 1;
     putbuf(ofs);

     tss.ph_sta-=ofs;
     for(j=0; j < tss.c_len - ofs; j++) {
       tss.chpho[j] = tss.chpho[j+ofs];
#if 0
	   if (!(tss.chpho[j].flag & FLAG_CHPHO_PHO_PHRASE))
		 tss.chpho[j].ch = tss.chpho[j].cha;
#endif
     }
     tss.c_idx-=ofs;
     tss.c_len-=ofs;
     prbuf();
   }


   init_chpho_i(tss.c_len);

   if (tss.c_idx < tss.c_len) {
     for(j=tss.c_len-1; j>=tss.c_idx; j--) {
       tss.chpho[j+1] = tss.chpho[j];
#if 0
	   if (!(tss.chpho[j+1].flag & FLAG_CHPHO_PHO_PHRASE)) {
		 tss.chpho[j+1].ch = tss.chpho[j+1].cha;
//		 dbg("copy %d %s\n", j+1, tss.chpho[j+1].ch);
	   }
#endif
     }
   }

   tss.c_len++;
   compact_win0();

#if 0
   prbuf();
   dbg("leave shift_ins\n");
#endif
}


static void put_u8_char(int pho_idx, phokey_t key, gboolean b_tone)
{
   shift_ins();
   int is_phrase;
   char *str = pho_idx_str2(pho_idx, &is_phrase);

   init_chpho_i(tss.c_idx);

//   dbg("put_b5_char %d] %d\n", tss.c_idx, b_tone);

   if (is_phrase) {
     dbg("is_phrase %s\n", str);
     tss.chpho[tss.c_idx].ch = str;
     tss.chpho[tss.c_idx].flag |= FLAG_CHPHO_PHO_PHRASE;
   }
   else {
     bzero(tss.chpho[tss.c_idx].cha, sizeof(tss.chpho[0].cha));
     bchcpy(tss.chpho[tss.c_idx].cha, str);
     tss.chpho[tss.c_idx].ch = tss.chpho[tss.c_idx].cha;
//     dbg("wwww %s\n",tss.chpho[tss.c_idx].ch);
   }

   if (b_tone)
     tss.chpho[tss.c_idx].flag |= FLAG_CHPHO_PINYIN_TONE;

   disp_char_chbuf(tss.c_idx);

   tss.chpho[tss.c_idx].pho=key;
   tss.c_idx++;

#if 0
   if (tss.c_idx < tss.c_len) {
     prbuf();
   }
#endif
}


#define MAX_PHRASE_SEL_N 10

static u_char selstr[MAX_PHRASE_SEL_N][MAX_PHRASE_LEN * CH_SZ];
static u_char sellen[MAX_PHRASE_SEL_N];

static u_short phrase_count;
static u_short pho_count;

static gboolean chpho_eq_pho(int idx, phokey_t *phos, int len)
{
  int i;

  for(i=0; i < len; i++)
    if (tss.chpho[idx+i].pho != phos[i])
       return FALSE;

  return TRUE;
}


char *get_chpho_pinyin_set(char *set_arr)
{
  if (!pin_juyin)
    return NULL;
  int i;
  for(i=0; i < tss.c_len; i++) {
    if (tss.chpho[i].flag & FLAG_CHPHO_PINYIN_TONE)
      set_arr[i]=TRUE;
    else
      set_arr[i]=FALSE;

//    dbg("pin %d] %d\n", i, set_arr[i]);
  }
  return set_arr;
}


static void get_sel_phrase0(int selidx, gboolean eqlen)
{
  int sti,edi;
  u_char len, mlen;

  mlen=tss.c_len-selidx;

  if (!mlen)
    return;

  if (mlen > MAX_PHRASE_LEN)
    mlen=MAX_PHRASE_LEN;

  phokey_t pp[MAX_PHRASE_LEN + 1];
  extract_pho(selidx, mlen, pp);

  char *pinyin_s = NULL;
  char pinyin_set[MAX_PH_BF_EXT];

  if (pin_juyin)
    pinyin_s = get_chpho_pinyin_set(pinyin_set) + selidx;

  if (!tsin_seek(pp, 2, &sti, &edi, pinyin_s))
    return;

  while (sti < edi && phrase_count < phkbm.selkeyN) {
    phokey_t stk[MAX_PHRASE_LEN];
    usecount_t usecount;
    u_char stch[MAX_PHRASE_LEN * CH_SZ + 1];

    load_tsin_entry(sti, (char *)&len, &usecount, stk, stch);
    mask_tone(stk, mlen, pinyin_s);

    if (eqlen && len!=mlen || (!eqlen && len > mlen) || len==1) {
      sti++;
      continue;
    }

    if (chpho_eq_pho(selidx, stk, len)) {
      sellen[phrase_count]=len;
      utf8cpyN((char *)selstr[phrase_count++], (char *)stch, len);
    }

    sti++;
  }
}

static void get_sel_phrase_end()
{
  int stidx = tss.c_idx - 5;
  if (stidx < 0)
    stidx = 0;

  phrase_count = 0;
  int i;
  for(i=stidx; i < tss.c_len - 1; i++) {
    get_sel_phrase0(i, TRUE);
  }
}

static void get_sel_phrase()
{
  phrase_count = 0;
  get_sel_phrase0(tss.c_idx, FALSE);
}

static void get_sel_pho()
{
  int idx = tss.c_idx==tss.c_len?tss.c_idx-1:tss.c_idx;
  phokey_t key = tss.chpho[idx].pho;

  if (!key)
    return;

  char need_mask = pin_juyin && !(tss.chpho[idx].flag & FLAG_CHPHO_PINYIN_TONE);
//  dbg("need_mask %d\n", need_mask);

  int i=hash_pho[key>>9];
  phokey_t ttt;

  while (i<idxnum_pho) {
    ttt=idx_pho[i].key;
    if (need_mask)
      ttt &= ~7;

    if (ttt>=key)
      break;
    i++;
  }

  if (ttt!=key) {
    return;
  }

  tss.startf = idx_pho[i].start;
  int end;

  if (need_mask) {
    while (i<idxnum_pho) {
      ttt=idx_pho[i].key;

      if (need_mask)
        ttt &= ~7;

      if (ttt>key)
        break;
      i++;
    }
    end = idx_pho[i].start;
//    dbg("end %d\n", i);
  } else
    end = idx_pho[i+1].start;

  pho_count = end - tss.startf;
//  dbg("pho_count %d\n", pho_count);
}


void clear_sele();
void set_sele_text(int tN, int i, char *text, int len);
void disp_arrow_up(), disp_arrow_down();
void disp_tsin_select(int index);

static void disp_current_sel_page()
{
  int i;

  clear_sele();

  for(i=0; i < phkbm.selkeyN; i++) {
    int idx = tss.current_page + i;

    if (idx < phrase_count) {
      int tlen = utf8_tlen((char *)selstr[i], sellen[i]);
      set_sele_text(phrase_count + pho_count, i, (char *)selstr[i], tlen);
    } else
    if (idx < phrase_count + pho_count) {
      int v = idx - phrase_count + tss.startf;
      char *tstr = pho_idx_str(v);
      set_sele_text(phrase_count + pho_count, i, tstr, -1);
    } else
      break;
  }

  if (tss.current_page + phkbm.selkeyN < phrase_count + pho_count) {
    disp_arrow_down();
  }

  if (tss.current_page > 0)
    disp_arrow_up();

  disp_tsin_select(tss.c_idx==tss.c_len?tss.c_idx-1:tss.c_idx);
}

static int fetch_user_selection(int val, char **seltext, int *is_pho_phrase)
{
  int idx = tss.current_page + val;
  int len = 0;

  *is_pho_phrase = FALSE;
  if (idx < phrase_count) {
    len = sellen[idx];
    *seltext = (char *)selstr[idx];
  } else
  if (idx < phrase_count + pho_count) {
    int v = idx - phrase_count + tss.startf;
    *seltext = pho_idx_str2(v, is_pho_phrase);
    len = utf8_str_N(*seltext);
  }

  return len;
}


void extract_pho(int chpho_idx, int plen, phokey_t *pho)
{
  int i;

  for(i=0; i < plen; i++) {
    pho[i] = tss.chpho[chpho_idx + i].pho;
  }
}


gboolean check_fixed_mismatch(int chpho_idx, char *mtch, int plen)
{
  int j;
  char *p = mtch;

  for(j=0; j < plen; j++) {
    int u8sz = utf8_sz(p);
    if (!(tss.chpho[chpho_idx+j].flag & FLAG_CHPHO_FIXED))
      continue;

    if (memcmp(tss.chpho[chpho_idx+j].ch, p, u8sz))
      return TRUE;

    p+= u8sz;
  }

  return FALSE;
}

#if 0
static u_char scanphr(int chpho_idx, int plen, gboolean pho_incr)
{
  return scanphr_e(chpho_idx, plen, pho_incr, NULL);
}
#endif

void hide_selections_win();

void disp_pre_sel_page()
{
  int i;

  if (!tsin_phrase_pre_select) {
    return;
  }

  if (!tss.pre_selN)
    return;

  clear_sele();

  for(i=0; i < tss.pre_selN; i++) {
    int tlen = utf8_tlen(tss.pre_sel[i].str, tss.pre_sel[i].len);

    set_sele_text(tss.pre_selN, i, tss.pre_sel[i].str, tlen);
  }

#if 0
  dbg("tss.ph_sta:%d\n", tss.ph_sta);
#endif
  disp_tsin_select(tss.ph_sta);
}

static void close_selection_win()
{
  hide_selections_win();
  tss.current_page=tss.sel_pho=tss.ctrl_pre_sel = 0;
  tss.pre_selN = 0;
}

void show_button_pho(gboolean bshow);

void tsin_set_eng_ch(int nmod)
{
//  dbg("tsin_set_eng_ch %d\n", nmod);
#if WIN32
  if (test_mode)
    return;
#endif
  if (current_CS) {
    current_CS->tsin_pho_mode = nmod;
    save_CS_current_to_temp();
  }

  if (current_method_type()==method_type_TSIN) {
    show_tsin_stat();
    drawcursor();

    if (!tsin_pho_mode())
      clrin_pho_tsin();

    show_button_pho(tsin_pho_mode());
  }

  show_tsin_stat();
}

void tsin_toggle_eng_ch()
{
//  dbg("tsin_toggle_eng_ch\n");
  compact_win0();
  tsin_set_eng_ch(!tsin_pho_mode());
}


void tsin_toggle_half_full()
{
    tss.tsin_half_full^=1;
    key_press_time = 0;
    drawcursor();
#if TRAY_ENABLED
    disp_tray_icon();
#endif
}


#if 0
static char ochars[]="<,>.?/:;\"'{[}]_-+=|\\~`";
#else
static char ochars[]="<,>.?/:;\"'{[}]_-+=|\\";
#endif

void hide_pre_sel()
{
  tss.pre_selN = 0;
  hide_selections_win();
}


static void call_tsin_parse()
{
  prbuf();
  tsin_parse();
  prbuf();
}

void disp_ph_sta_idx(int idx)
{
}

void disp_ph_sta()
{
  disp_ph_sta_idx(tss.ph_sta);
}

void ch_pho_cpy(CHPHO *pchpho, char *utf8, phokey_t *phos, int len)
{
  int i;

  for(i=0; i < len; i++) {
    int len = utf8cpy(pchpho[i].cha, utf8);
    utf8+=len;
    pchpho[i].pho = phos[i];
	pchpho[i].flag &= ~FLAG_CHPHO_PHO_PHRASE;
  }
}


void set_chpho_ch(CHPHO *pchpho, char *utf8, int len, gboolean is_pho_phrase)
{
  int i;

  for(i=0; i < len; i++) {
    int u8len;
    if (is_pho_phrase) {
      pchpho[i].ch = utf8;
	  pchpho[i].flag |= FLAG_CHPHO_PHO_PHRASE;
	} else {
      u8len = utf8cpy(pchpho[i].cha, utf8);
      pchpho[i].ch = pchpho[i].cha;
	  pchpho[i].flag &= ~FLAG_CHPHO_PHO_PHRASE;
    }

    utf8+=u8len;
  }
}


#if USE_TSIN
gboolean add_to_tsin_buf(char *str, phokey_t *pho, int len)
{
    int i;

    if (tss.c_idx < 0 || tss.c_len + len >= MAX_PH_BF_EXT)
      return 0;

    if (tss.c_idx < tss.c_len) {
      for(i=tss.c_len-1; i >= tss.c_idx; i--) {
        tss.chpho[i+len] = tss.chpho[i];
      }
    }

    ch_pho_cpy(&tss.chpho[tss.c_idx], str, pho, len);

    if (tss.c_idx == tss.c_len)
      tss.c_idx +=len;

    tss.c_len+=len;

    clrin_pho_tsin();
    disp_in_area_pho_tsin();

    prbuf();

    set_fixed(tss.c_idx, len);
#if 1
    for(i=1;i < len; i++) {
      tss.chpho[tss.c_idx+i].psta= tss.c_idx;
    }
#endif
#if 0
    if (len > 0)
      tss.chpho[tss.c_idx].flag |= FLAG_CHPHO_PHRASE_HEAD;
#endif
    drawcursor();
    disp_ph_sta();
    hide_pre_sel();
    tss.ph_sta=-1;

    if (hime_pop_up_win)
      show_win0();

    return TRUE;
}
#endif

#if 1
static void set_phrase_link(int idx, int len)
{
    int j;

    if (len < 1)
      return;

    for(j=1;j < len; j++) {
      tss.chpho[idx+j].psta=idx;
    }

    tss.chpho[idx].flag |= FLAG_CHPHO_PHRASE_HEAD;
}
#endif


// should be used only if it is a real phrase
gboolean add_to_tsin_buf_phsta(char *str, phokey_t *pho, int len)
{
    int idx = tss.ph_sta;
#if 0
    dbg("idx:%d  tss.ph_sta:%d tss.ph_sta_last:%d tss.c_idx:%d  tss.c_len:%d\n",
       idx, tss.ph_sta, tss.ph_sta_last, tss.c_idx, tss.c_len);
#endif
    if (idx < 0)
      return 0;

    if (idx + len >= MAX_PH_BF_EXT)
      flush_tsin_buffer();

    if (tss.c_idx < tss.c_len) {
      int avlen = tss.c_idx - tss.ph_sta;
//      dbg("avlen:%d %d\n", avlen, len);
      if (avlen < len) {
        int d = len - avlen;

        memmove(&tss.chpho[tss.c_idx + d], &tss.chpho[tss.c_idx], sizeof(CHPHO) * (tss.c_len - tss.c_idx));
        tss.c_len += d;
      }
    } else
      tss.c_len = idx + len;

    ch_pho_cpy(&tss.chpho[idx], str, pho, len);
    set_chpho_ch(&tss.chpho[idx], str, len, FALSE);
    set_fixed(idx, len);
    tss.chpho[idx].flag |= FLAG_CHPHO_PHRASE_USER_HEAD;
    tss.c_idx=idx + len;
    tss.chpho[tss.c_idx - 1].flag |= FLAG_CHPHO_PHRASE_TAIL;

    clrin_pho_tsin();
    disp_in_area_pho_tsin();

    prbuf();
#if 1
    set_phrase_link(idx, len);
#endif
    drawcursor();
    disp_ph_sta();
    hide_pre_sel();
    tss.ph_sta=-1;
    call_tsin_parse();

    return 1;
}


void add_to_tsin_buf_str(char *str)
{
  char *pp = str;
  char *endp = pp+strlen(pp);
  int N = 0;


  while (*pp) {
    int u8sz = utf8_sz(pp);
    N++;
    pp += u8sz;

    if (pp >= endp) // bad utf8 string
      break;
  }

  dbg("add_to_tsin_buf_str %s %d\n",str, N);

  phokey_t pho[MAX_PHRASE_LEN];
  bzero(pho, sizeof(pho));
  add_to_tsin_buf(str, pho, N);
}

int tsin_pho_sel(int c);

int tsin_sele_by_idx(int c)
{
  if (tss.sel_pho) {
    tsin_pho_sel(c);
    return 0;
  }

  int len = tss.pre_sel[c].len;

#if 0
    dbg("eqlenN:%d %d\n", c, tss.pre_selN);
#endif

  if (c >= tss.pre_selN)
    return 0;

  tss.full_match = FALSE;
  gboolean b_added = add_to_tsin_buf_phsta(tss.pre_sel[c].str, (phokey_t*)tss.pre_sel[c].phkey, len);

  return b_added;
}

static char shift_sele[]="!@#$%^&*()asdfghjkl:zxcvbnm<>?qwertyuiop";
static char noshi_sele[]="1234567890asdfghjkl;zxcvbnm,./qwertyuiop";
int shift_key_idx(char *s, KeySym xkey)
{
  if (xkey >= 0x7f)
    return -1;

  if (isupper(xkey))
    xkey = xkey - 'A' + 'a';

//  dbg("pre_sel_handler aa\n");

  char *p;
  if (!(p=strchr(shift_sele, xkey)))
    return -1;

  int c = p - shift_sele;
  char noshi = noshi_sele[c];

  if (!(p=strchr(s, noshi)))
    return -1;

  c = p - s;
  return c;
}


static gboolean pre_sel_handler(KeySym xkey)
{
  if (!tss.pre_selN || !tsin_phrase_pre_select)
    return FALSE;

  int c = shift_key_idx(pho_selkey, xkey);
  if (c < 0) {
    close_selection_win();
    return FALSE;
  }
  return tsin_sele_by_idx(c);
}

static gboolean pre_punctuation_sub(KeySym xkey, char shift_punc[], unich_t *chars[])
{
  char *p;
  if (xkey > 0x7e)
    return FALSE;

  if ((p=strchr(shift_punc, xkey))) {
    int c = p - shift_punc;
    char *pchar = _(chars[c]);

    if (current_method_type() == method_type_PHO) {
      char tt[CH_SZ+1];
      utf8cpy(tt, pchar);
      send_text(tt);
    } else {
      phokey_t keys[64];
      keys[0]=0;
      utf8_pho_keys(pchar, keys);
      add_to_tsin_buf(pchar, &keys[0], 1);
      if (tsin_cursor_end())
        flush_tsin_buffer();
    }
    return 1;
  }

  return 0;
}


gboolean pre_punctuation(KeySym xkey)
{
  static char shift_punc[]="<>?:\"{}!_";
  static unich_t *chars[]={_L("，"),_L("。"),_L("？"),_L("："),_L("；"),_L("『"),_L("』"),_L("！"),_L("——")};
  return pre_punctuation_sub(xkey, shift_punc, chars);
}

static char hsu_punc[]=",./;'";
gboolean pre_punctuation_hsu(KeySym xkey)
{
  static unich_t *chars[]={_L("，"),_L("。"),_L("？"),_L("；"),_L("、")};
  return pre_punctuation_sub(xkey, hsu_punc, chars);
}


int inph_typ_pho(KeySym newkey);
gint64 current_time();


KeySym keypad_proc(KeySym xkey)
{
  if (xkey <= XK_KP_9 && xkey >= XK_KP_0)
    xkey=xkey-XK_KP_0+'0';
  else {
    switch (xkey) {
      case XK_KP_Add:
        xkey = '+';
        break;
      case XK_KP_Subtract:
        xkey = '-';
        break;
      case XK_KP_Multiply:
        xkey = '*';
        break;
      case XK_KP_Divide:
        xkey = '/';
        break;
      case XK_KP_Decimal:
        xkey = '.';
        break;
      default:
        return 0;
    }
  }

  return xkey;
}

static int cursor_left()
{
//  dbg("cursor left %d %d\n", tss.c_idx, tss.c_len);
  close_selection_win();
  if (tss.c_idx) {
    clrcursor();
    tss.c_idx--;
    drawcursor();
    return 1;
  }
  // Thanks to PCMan.bbs@bbs.sayya.org for the suggestion
  return tss.c_len;
}
static int cursor_right()
{
//  dbg("cursor right %d %d\n", tss.c_idx, tss.c_len);
  close_selection_win();
  if (tss.c_idx < tss.c_len) {
    clrcursor();
    tss.c_idx++;
    drawcursor();
    return 1;
  }

  return tss.c_len;
}

void tsin_scan_pre_select(gboolean b_incr);

static int cursor_backspace()
{
        close_selection_win();
        poo.ityp3_pho=0;
        tss.pre_selN = 0;
        gboolean pho_cleared;
        pho_cleared=FALSE;
		int j;

        if (pin_juyin) {
          for(j=sizeof(poo.inph)-1;j>=0;j--) {
            if (poo.inph[j]) {
              poo.inph[j]=0;
              pho_cleared = TRUE;
			  if (j==0)
				clrin_pho();
              break;
            }
          }
        } else {
          for(j=3;j>=0;j--)
            if (poo.typ_pho[j]) {
              poo.typ_pho[j]=0;
              poo.inph[j]=0;
              pho_cleared = TRUE;
              break;
            }
        }

        if (pho_cleared) {
//          dbg("pho cleared %d %d %d\n",tss.c_len, hime_pop_up_win, typ_pho_empty());
		  if (typ_pho_empty())
			  bzero(poo.inph, sizeof(poo.inph));

          disp_in_area_pho_tsin();
          tsin_scan_pre_select(TRUE);

          if (!tss.c_len && hime_pop_up_win && typ_pho_empty())
            hide_win0();
          return 1;
        }

        if (!tss.c_idx)
          return 0;

        clrcursor();
        tss.c_idx--;
//        pst=k=tss.chpho[tss.c_idx].psta;

		int k;
        for(k=tss.c_idx;k<tss.c_len;k++) {
          tss.chpho[k]=tss.chpho[k+1];
          if (tss.chpho[k+1].ch == tss.chpho[k+1].cha)
            tss.chpho[k].ch = tss.chpho[k].cha;
        }

        tss.c_len--;
        init_chpho_i(tss.c_len);
		call_tsin_parse();
		compact_win0();

        if (!tss.c_idx) {
          clear_match();
        } else {
          tsin_scan_pre_select(TRUE);
        }

        disp_ph_sta();

        if (!tss.c_len && hime_pop_up_win)
          hide_win0();

        return 1;
}


static int cursor_delete()
{
	if (tss.c_idx>=tss.c_len)
		return FALSE;
	tss.c_idx++;
	return cursor_backspace();
}

void case_inverse(KeySym *xkey, int shift_m);
void pho_play(phokey_t key);

int tsin_pho_sel(int c)
{
  char *sel_text;
  int is_pho_phrase;
  int len = fetch_user_selection(c, &sel_text, &is_pho_phrase);
  int sel_idx = tss.c_idx;
  if (tss.c_idx == tss.c_len)
    sel_idx = tss.c_len - len;

  set_chpho_ch(&tss.chpho[sel_idx], sel_text, len, is_pho_phrase);

  set_fixed(sel_idx, len);

  call_tsin_parse();

  if (tss.c_idx + len == tss.c_len) {
    tss.ph_sta = -1;
//    draw_ul(tss.c_idx, tss.c_len);
  }

  if (len) {
    prbuf();
    tss.current_page=tss.sel_pho=poo.ityp3_pho=0;
    if (len == 1) {
      hide_selections_win();
      tss.ph_sta = -1;
      return 0;
    }
    else
      tss.ph_sta=-1;

    hide_selections_win();
  }

  return 1;
}


gboolean tsin_page_up()
{
  if (!tss.sel_pho)
    return tss.c_len;

  tss.current_page = tss.current_page - phkbm.selkeyN;
  if (tss.current_page < 0)
    tss.current_page = 0;

  tss.pho_menu_idx = 0;
  disp_current_sel_page();
  return TRUE;
}

gboolean tsin_page_down()
{
  if (!tss.sel_pho)
    return tss.c_len;

  tss.pho_menu_idx = 0;
  tss.current_page = tss.current_page + phkbm.selkeyN;
  if (tss.current_page >= phrase_count + pho_count)
    tss.current_page = 0;

  disp_current_sel_page();

  return TRUE;
}

void open_select_pho()
{
  if (tss.c_idx==tss.c_len) {
    get_sel_phrase_end();
  } else
    get_sel_phrase();

  get_sel_pho();
  tss.sel_pho=1;
  tss.pho_menu_idx = tss.current_page = 0;
  disp_current_sel_page();
}

gboolean win_sym_page_up(), win_sym_page_down();

static void tsin_create_win_save_phrase(int idx0, int len)
{
  WSP_S wsp[MAX_PHRASE_LEN];
  int i;
  for(i=0;i<len;i++) {
    memcpy(wsp[i].ch, tss.chpho[i+idx0].ch, CH_SZ);
    wsp[i].key = tss.chpho[i+idx0].pho;
  }
  create_win_save_phrase(wsp, len);
}


int feedkey_pp(KeySym xkey, int kbstate)
{
  char ctyp=0;
  static u_int ii;
  static u_short key;
  int shift_m=kbstate&ShiftMask;
  int ctrl_m=kbstate&ControlMask;
  int jj,kk, idx;
  char kno;
  int caps_eng_tog = tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock;
  int status=0;


//  dbg("feedkey_pp %x %x\n", xkey, kbstate);
//  if (xkey=='1')
//    dbg("aaa\n");

  if (caps_eng_tog) {
    gboolean new_tsin_pho_mode =!(kbstate&LockMask);
    if (current_CS->tsin_pho_mode != new_tsin_pho_mode) {
      close_selection_win();
      tsin_set_eng_ch(new_tsin_pho_mode);
    }
  }

//  key_press_time = 0;

   if (kbstate & (Mod1Mask|Mod4Mask|Mod5Mask)) {
//     dbg("ret\n");
     return 0;
   }

   // Shift has autorepeat on win32
   if ((xkey==XK_Shift_L||xkey==XK_Shift_R) && !key_press_time) {
//	  dbg("feedkey_pp\n");
     key_press_time = current_time();
     key_press_time_ctrl = 0;
   } else
   if ((xkey==XK_Control_L||xkey==XK_Control_R)
	   && !key_press_time_ctrl && tss.pre_selN) {
//	  dbg("feedkey_pp\n");
     key_press_time_ctrl = current_time();
     key_press_time = 0;
     return TRUE;
   } else {
     key_press_time = 0;
     key_press_time_ctrl = 0;
   }

   if (!tsin_pho_mode() && !tss.c_len && hime_pop_up_win && xkey!=XK_Caps_Lock) {
     hide_win0();
     gboolean is_ascii = (xkey>=' ' && xkey<0x7f) && !ctrl_m;

     if (caps_eng_tog && is_ascii) {
       if (hime_capslock_lower)
         case_inverse(&xkey, shift_m);
       send_ascii(xkey);
       return 1;
     }
     else {
       if (tss.tsin_half_full && is_ascii) {
         send_text(half_char_to_full_char(xkey));
         return 1;
       }
       else {
         return 0;
       }
     }
   }

   int o_sel_pho = tss.sel_pho;
   close_win_pho_near();

   switch (xkey) {
     case XK_Escape:
       tsin_reset_in_pho0();
       if (typ_pho_empty()) {
         if (!tss.c_len)
           return 0;
         if (!o_sel_pho && tsin_tab_phrase_end) {
           goto tab_phrase_end;
         }
       }
       tsin_reset_in_pho();
       return 1;
     case XK_Return:
     case XK_KP_Enter:
        if (shift_m) {
          if (!tss.c_len)
            return 0;
          tsin_create_win_save_phrase(tss.c_idx,  tss.c_len - tss.c_idx);
          move_cursor_end();
          return 1;
        } else {
          if (tss.sel_pho) {
            tsin_sele_by_idx(tss.pho_menu_idx);
          } else {
            if (tss.c_len)
              flush_tsin_buffer();
            else
            if (typ_pho_empty())
              return 0;
          }
          return 1;
        }
     case XK_Home:
#if UNIX
     case XK_KP_Home:
#endif
        close_selection_win();
        if (!tss.c_len)
          return 0;
        clrcursor();
        tss.c_idx=0;
        drawcursor();
        return 1;
     case XK_End:
#if UNIX
     case XK_KP_End:
#endif
        close_selection_win();
        if (!tss.c_len)
          return 0;
        move_cursor_end();
        return 1;
     case XK_Left:
#if UNIX
     case XK_KP_Left:
#endif
        return cursor_left();
     case XK_Right:
#if UNIX
     case XK_KP_Right:
#endif
        return cursor_right();
     case XK_Caps_Lock:
        if (caps_eng_tog) {
#if 0
          close_selection_win();
#if UNIX
          tsin_toggle_eng_ch();
#else
		  tsin_set_eng_ch(!(kbstate&LockMask));
#endif
#endif
          return 1;
        } else
          return 0;
     case XK_Tab:
        close_selection_win();
        if (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Tab) {
          tsin_toggle_eng_ch();
          return 1;
        }

        if (tsin_tab_phrase_end && tss.c_len > 1) {
tab_phrase_end:
          if (tss.c_idx==tss.c_len)
            tss.chpho[tss.c_idx-1].flag |= FLAG_CHPHO_PHRASE_USER_HEAD;
          else
            tss.chpho[tss.c_idx].flag |= FLAG_CHPHO_PHRASE_USER_HEAD;
           call_tsin_parse();
          return 1;
        } else {
          if (tss.c_len) {
            flush_tsin_buffer();
            return 1;
          }
        }
        return 0;
     case XK_Delete:
#if UNIX
     case XK_KP_Delete:
#endif
        return cursor_delete();
     case XK_BackSpace:
		return cursor_backspace();
     case XK_Up:
#if UNIX
     case XK_KP_Up:
#endif
       if (!tss.sel_pho) {
         if (tsin_use_pho_near && tss.c_len && tss.c_idx == tss.c_len) {
           int idx = tss.c_len-1;
           phokey_t pk = tss.chpho[idx].pho;

           if (pk) {
             void create_win_pho_near(phokey_t pho);
             create_win_pho_near(pk);
           }

           return 1;
         }
         return 0;
       }

       int N;
       N = phrase_count + pho_count - tss.current_page;
       if (N > phkbm.selkeyN)
         N = phkbm.selkeyN;
       tss.pho_menu_idx--;
       if (tss.pho_menu_idx < 0)
         tss.pho_menu_idx = N-1;
       disp_current_sel_page();
       return 1;
     case XK_Prior:
#if UNIX
     case XK_KP_Prior:
#endif
     case XK_KP_Subtract:
       if (!tss.sel_pho && tss.c_len && xkey == XK_KP_Subtract) {
         add_to_tsin_buf_str("-");
         return TRUE;
       } else {
         if (tss.c_len && !tss.sel_pho)
           return win_sym_page_up();
         if (tsin_page_up())
           return TRUE;
         return win_sym_page_up();
       }
     case XK_space:
       if (!tss.c_len && !poo.ityp3_pho && !poo.typ_pho[0] && !poo.typ_pho[1] && !poo.typ_pho[2]
           && tss.tsin_half_full) {
         send_text(_(_L("　")));
         return 1;
       }

       if (tsin_space_opt == TSIN_SPACE_OPT_INPUT && !poo.typ_pho[0] && !poo.typ_pho[1] && !poo.typ_pho[2] && !poo.ityp3_pho && !tss.sel_pho) {
         if (tss.c_len)
           flush_tsin_buffer();

         close_selection_win();
         goto asc_char;
       }

       if (!tsin_pho_mode())
           goto asc_char;
     case XK_Down:
#if UNIX
     case XK_KP_Down:
#endif
       if (xkey==XK_space && !poo.ityp3_pho && (poo.typ_pho[0]||poo.typ_pho[1]||poo.typ_pho[2])) {
         kno=0;
#if 1
         ctyp=3;
//         status = inph_typ_pho(xkey);
#endif
         goto llll1;
       }

change_char:
       if (!tss.c_len)
         return 0;

       idx = tss.c_idx==tss.c_len ? tss.c_idx - 1 : tss.c_idx;
       if (!tss.chpho[idx].pho)
         return 1;

       if (!tss.sel_pho) {
         open_select_pho();
       } else {
         if (xkey == XK_space)
           tsin_page_down();
         else {
           int N = phrase_count + pho_count - tss.current_page;
           if (N > phkbm.selkeyN)
             N = phkbm.selkeyN;
           tss.pho_menu_idx = (tss.pho_menu_idx+1) % N;
           disp_current_sel_page();
         }
       }
       return 1;
     case XK_Next:
#if UNIX
     case XK_KP_Next:
#endif
     case XK_KP_Add:
       if (!tss.sel_pho && tss.c_len && xkey == XK_KP_Add) {
         add_to_tsin_buf_str("+");
         return TRUE;
       } else {
         if (tss.c_len && !tss.sel_pho)
           return win_sym_page_down();
		 if (tsin_page_down())
           return TRUE;
		 return win_sym_page_down();
	   }
     case '\'':  // single quote
       if (phkbm.phokbm[xkey][0].num && !pin_juyin)
         goto other_keys;
       else {
	 return pre_punctuation_hsu(xkey);
       }
     case 'q':
     case 'Q':
       if (b_hsu_kbm && tsin_pho_mode())
         goto change_char;
     default:
other_keys:
       if ((kbstate & ControlMask)) {
         if (xkey=='u') {
           if (tss.c_len) {
             clear_tsin_buffer();
             if (hime_pop_up_win)
               hide_win0();
             return 1;
           } else
             return 0;
         } else if (tsin_buffer_editing_mode && xkey == 'e') { //ctrl+e only works when user enabled tsin_buffer_editing_mode
           //toggler
           tss.tsin_buffer_editing ^= 1;
           return 1;
         } else if (xkey>='1' && xkey<='9') {
           if (!tss.c_len)
             return 0;
           if (!tss.c_idx)
             return 1;

           int len = xkey - '0';
           int idx0 = tss.c_idx - len;

           if (idx0 < 0)
             return 1;

           tsin_create_win_save_phrase(idx0, len);
           return 1;
         } else {
           return 0;
         }
       }

       char xkey_lcase = xkey;
       if ('A' <= xkey && xkey <= 'Z') {
         xkey_lcase = tolower(xkey);
       }


       if (tsin_buffer_editing_mode && xkey == '\\') {
         tss.tsin_buffer_editing ^= 1;
         if (tss.tsin_buffer_editing && tss.c_idx==tss.c_len)
           cursor_left();
         return TRUE;
       }

       if (!tss.c_len)
         tss.tsin_buffer_editing = FALSE;

       if (tss.tsin_buffer_editing && !tss.sel_pho) {
         if (xkey_lcase=='h' || xkey_lcase=='j')
           return cursor_left();
         else
         if (xkey_lcase=='l' || xkey_lcase=='k')
           return cursor_right();
         else
         if (xkey_lcase=='x')
           return cursor_delete();
         else
           return TRUE;
       }

       if (xkey >= XK_KP_0 && xkey<=XK_KP_9)
         xkey_lcase = xkey - XK_KP_0 + '0';

       gboolean use_pre_sel;
       use_pre_sel = tss.pre_selN && !tss.sel_pho && xkey < 127 && !phkbm.phokbm[xkey][0].num;

       char *pp;
       if ((pp=strchr(pho_selkey,xkey_lcase)) && (tss.sel_pho || tss.ctrl_pre_sel || use_pre_sel)) {
         int c=pp-pho_selkey;

         if (tss.sel_pho) {
           if (tsin_pho_sel(c))
             return 1;
         } else
         if (tss.ctrl_pre_sel || use_pre_sel) {
           tss.ctrl_pre_sel = FALSE;
           if (tsin_sele_by_idx(c))
             return TRUE;
           else {
             close_selection_win();
           }
         }

         goto scan_it;
       }

       tss.sel_pho=tss.current_page=0;
   }

   KeySym key_pad;
   key_pad = keypad_proc(xkey);

   if (!xkey || (xkey > 0x7e && !key_pad))
     return 0;

   if (key_pad && !tss.c_len && !tss.tsin_half_full)
     return 0;

   if (!tsin_pho_mode() || poo.typ_pho[0]!=BACK_QUOTE_NO && (shift_m || key_pad ||
       (!phkbm.phokbm[xkey][0].num && !phkbm.phokbm[xkey][0].typ))) {
       if (tsin_pho_mode() && !shift_m && strchr(hsu_punc, xkey) && !phkbm.phokbm[xkey][0].num) {
         if (pre_punctuation_hsu(xkey))
           return 1;
       }

       if (key_pad)
         xkey = key_pad;
asc_char:
        if (shift_m) {
          if (pre_sel_handler(xkey)) {
            call_tsin_parse();
            return 1;
          }

          if (tsin_pho_mode() && pre_punctuation(xkey))
            return 1;
        }

        if (shift_m && tsin_pho_mode())  {
          char *ppp=strchr(ochars,xkey);

          if (!(kbstate&LockMask) && ppp && !((ppp-ochars) & 1))
            xkey=*(ppp+1);

        } else {
          if (!tsin_pho_mode() && tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock
              && hime_capslock_lower) {
            case_inverse(&xkey, shift_m);
          }
        }

        if (xkey > 127)
          return 0;
        char tstr[CH_SZ + 1];
        bzero(tstr, sizeof(tstr));

        u_char tt=xkey;

        if (tss.tsin_half_full) {
          strcpy(tstr, half_char_to_full_char(xkey));
        } else {
          tstr[0] = tt;
        }

        if (!tss.c_len) {
          send_text(tstr);
          return 1;
        }

        shift_ins();

        memcpy(tss.chpho[tss.c_idx].ch, tstr, CH_SZ);

        set_fixed(tss.c_idx, 1);
        phokey_t tphokeys[32];
        tphokeys[0]=0;
        utf8_pho_keys(tss.chpho[tss.c_idx].ch, tphokeys);

        disp_char_chbuf(tss.c_idx);
        tss.chpho[tss.c_idx].pho=tphokeys[0];
        tss.c_idx++;
        if (tss.c_idx < tss.c_len)
          prbuf();

        if (hime_pop_up_win)
          show_win0();

        drawcursor();
        return 1;
   }


     if (xkey > 127) {
       return 0;
     }

     // for hsu & et26
     if (xkey >= 'A' && xkey <='Z' && poo.typ_pho[0]!=BACK_QUOTE_NO)
       xkey+=0x20;
//     printf("bbbb %c\n", xkey);

llll1:
     status = inph_typ_pho(xkey);
     if (hime_pop_up_win)
         show_win0();

     if (poo.typ_pho[3] || (status&PHO_STATUS_OK_NEW))
       ctyp = 3;

//     dbg("status %d %d\n", status, ctyp);
     jj=0;
     kk=1;
llll2:
     if (ctyp==3) {
       poo.ityp3_pho=1;  /* last key is entered */

       if (!tsin_tone_char_input && !poo.typ_pho[0] && !poo.typ_pho[1] && !poo.typ_pho[2]) {
         clrin_pho_tsin();
         dbg("no pho input\n");
         return TRUE;
       }
     }

     disp_in_area_pho_tsin();

     key = pho2key(poo.typ_pho);

     pho_play(key);

     int vv=hash_pho[poo.typ_pho[0]];

     phokey_t ttt=0xffff;
     while (vv<idxnum_pho) {
       ttt=idx_pho[vv].key;
       if (poo.typ_pho[0]!=BACK_QUOTE_NO) {
         if (!poo.typ_pho[0]) ttt &= ~(31<<9);
         if (!poo.typ_pho[1]) ttt &= ~(3<<7);
         if (!poo.typ_pho[2]) ttt &= ~(15<<3);
         if (!poo.typ_pho[3]) ttt &= ~(7);
       }
       if (ttt>=key) break;
       else
       vv++;
     }
#if 0
     printf("aaaa vv:%d  idxnum_pho:%d   ttt:%x key:%x\n",vv, idxnum_pho, ttt, key);
#endif
     if (!pin_juyin && (ttt > key || (poo.ityp3_pho && idx_pho[vv].key!=key))) {
       while (jj<4) {
         while(kk<3)
         if (phkbm.phokbm[(int)poo.inph[jj]][kk].num ) {
           if (kk) {
             ctyp=phkbm.phokbm[(int)poo.inph[jj]][kk-1].typ;
             poo.typ_pho[(int)ctyp]=0;
           }
           kno=phkbm.phokbm[(int)poo.inph[jj]][kk].num;
           ctyp=phkbm.phokbm[(int)poo.inph[jj]][kk].typ;
           poo.typ_pho[(int)ctyp]=kno;
           kk++;
           goto llll2;
         } else kk++;
         jj++;
         kk=1;
       }

       bell(); poo.ityp3_pho=poo.typ_pho[3]=0;
       disp_in_area_pho_tsin();
//       dbg("not found ...\n");
       return 1;
     }

     if (poo.typ_pho[0]==L_BRACKET_NO||poo.typ_pho[0]==R_BRACKET_NO || poo.typ_pho[0]==BACK_QUOTE_NO && poo.typ_pho[1])
       poo.ityp3_pho = 1;

     if (key==0 || !poo.ityp3_pho) {
       if (key)
         tsin_scan_pre_select(TRUE);
//       dbg("ret a\n");
       return 1;
     }

     ii=idx_pho[vv].start;
     poo.start_idx=ii;
     poo.stop_idx = idx_pho[vv+1].start;
#if 0
     printf("%x %x %d vv:%d idxnum_pho:%d-->", ttt, key, poo.start_idx, vv, idxnum_pho);
     utf8_putchar(pho_idx_str(poo.start_idx));
     puts("<---");
#endif

   if (!tss.c_len && poo.typ_pho[0]==BACK_QUOTE_NO && poo.stop_idx - poo.start_idx == 1)
     send_text(pho_idx_str(poo.start_idx));  // it's ok since ,. are 3 byte, last one \0
   else
     put_u8_char(poo.start_idx, key, (status&PHO_STATUS_TONE)>0);

   call_tsin_parse();

   disp_ph_sta();
   if (status & PHO_STATUS_PINYIN_LEFT) {
     poo.ityp3_pho=0;
     disp_in_area_pho_tsin();
   } else {
     clrin_pho_tsin();
     clr_in_area_pho_tsin();
   }
   drawcursor();
   hide_pre_sel();

scan_it:
   tsin_scan_pre_select(FALSE);

   return 1;
}


int feedkey_pp_release(KeySym xkey, int kbstate)
{
  gint64 kpt;

  switch (xkey) {
     case XK_Shift_L:
     case XK_Shift_R:
		kpt = key_press_time;
		key_press_time = 0;

// dbg("release xkey %x\n", xkey);
        if (
(  (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Shift) ||
   (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftL
     && xkey == XK_Shift_L) ||
   (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftR
     && xkey == XK_Shift_R))
          &&  current_time() - kpt < 300000) {
          if (!test_mode) {
            close_selection_win();
            tsin_toggle_eng_ch();
          }
          return 1;
		} else
          return 0;
     case XK_Control_L:
     case XK_Control_R:
		kpt = key_press_time_ctrl;
		key_press_time_ctrl = 0;
        if (current_time() - kpt < 300000 && tss.pre_selN) {
          if (!test_mode) {
			tss.ctrl_pre_sel = TRUE;
          }
          return 1;
		} else
          return 0;
     default:
        return 0;
  }
}


#if USE_TSIN
void tsin_remove_last()
{
  if (!tss.c_len)
    return;
  tss.c_len--;
  tss.c_idx--;
}
#endif


gboolean save_phrase_to_db2(CHPHO *chph, int len)
{
   phokey_t pho[MAX_PHRASE_LEN];
   char ch[MAX_PHRASE_LEN * CH_SZ * 2];

   chpho_extract_cha(chph, len, pho, ch);

   return save_phrase_to_db(pho, ch, len, 1);
}

#include "im-client/hime-im-client-attr.h"


int tsin_get_preedit(char *str, HIME_PREEDIT_ATTR attr[], int *cursor, int *comp_flag)
{
  int i;
  int tn=0;
  int attrN=0;
#if DEBUG && 0
  dbg("tsin_get_preedit\n");
#endif

  gboolean ap_only = hime_edit_display_ap_only();

  for(i=0; i<tss.c_len; i++) {
    if (tn>=HIME_PREEDIT_MAX_STR-4*CH_SZ-1)
      goto fin;
    if (i==tss.c_idx && hime_display_on_the_spot_key()) {
      tn += get_in_area_pho_tsin_str(str+tn);
    }

    strcpy(str+tn, tss.chpho[i].ch);
    tn+=strlen(tss.chpho[i].ch);
  }

fin:
  str[tn]=0;

  if (i==tss.c_idx && hime_display_on_the_spot_key())
    get_in_area_pho_tsin_str(str+tn);

#if DEBUG && 0
  dbg("'%s'\n", str);
#endif
  if (tss.c_len) {
    attr[0].flag=HIME_PREEDIT_ATTR_FLAG_UNDERLINE;
    attr[0].ofs0=0;
    attr[0].ofs1=tss.c_len;
    attrN++;


    // for firefox 4
    if (ap_only && tss.c_idx < tss.c_len) {
      attr[1].ofs0=tss.c_idx;
      attr[1].ofs1=tss.c_idx+1;
      attr[1].flag=HIME_PREEDIT_ATTR_FLAG_REVERSE;
      attrN++;
    }
  }

  *cursor = tss.c_idx;
#if WIN32 || 1
  *comp_flag = !typ_pho_empty();
  if (gwin1 && GTK_WIDGET_VISIBLE(gwin1))
    *comp_flag|=2;
#if 1
  if (tss.c_len && !ap_only)
	*comp_flag|=4;
#endif
#endif

  return attrN;
}

int tsin_reset()
{
//  dbg("tsin_reset\n");
#if UNIX
  if (!gwin0)
    return 0;
#endif
  int v = tss.c_len > 0;
  tsin_reset_in_pho0();
  clear_tsin_buffer();

  return v;
}


#if WIN32
static TSIN_ST temp_st;
static PRE_SEL *temp_pre_sel;
static CHPHO *temp_chpho;
void pho_save_gst(),pho_restore_gst();

void tsin_save_gst()
{
  pho_save_gst();

  if (!temp_pre_sel)
    temp_pre_sel=tzmalloc(PRE_SEL, 10);

  if (!temp_chpho)
    temp_chpho=tzmalloc(CHPHO, MAX_PH_BF_EXT);


  temp_st = tss;
  memcpy(temp_chpho, tss.chpho, sizeof(CHPHO) * MAX_PH_BF_EXT);
  memcpy(temp_pre_sel, tss.pre_sel, sizeof(PRE_SEL) * 10);
}

void  tsin_restore_gst()
{
  pho_restore_gst();

  tss = temp_st;
  memcpy(tss.chpho, temp_chpho, sizeof(CHPHO) * MAX_PH_BF_EXT);
  memcpy(tss.pre_sel, temp_pre_sel, sizeof(PRE_SEL) * 10);
}
#endif
