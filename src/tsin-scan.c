/* Copyright (C) 1995-2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
#include "tsin.h"
#include "gtab.h"
#include "gst.h"

static int qcmp_pre_sel_usecount(const void *aa, const void *bb)
{
  PRE_SEL *a = (PRE_SEL *) aa;
  PRE_SEL *b = (PRE_SEL *) bb;

  return b->usecount - a->usecount;
}

static int qcmp_pre_sel_str(const void *aa, const void *bb)
{
  PRE_SEL *a = (PRE_SEL *) aa;
  PRE_SEL *b = (PRE_SEL *) bb;

  int d = strcmp(a->str, b->str);
  if (d)
    return d;

  return b->usecount - a->usecount;
}

void extract_gtab_key(int start, int len, void *out);
gboolean check_gtab_fixed_mismatch(int idx, char *mtch, int plen);
void mask_tone(phokey_t *pho, int plen, char *tone_off);
void init_pre_sel();
void mask_key_typ_pho(phokey_t *key);
extern u_int64_t vmaskci;

u_char scanphr_e(int chpho_idx, int plen, gboolean pho_incr, int *rselN)
{
  if (plen >= MAX_PHRASE_LEN)
    goto empty;
  if (chpho_idx < 0)
    goto empty;

  phokey_t tailpho;

  if (pho_incr) {
    if (ph_key_sz==2) {
      tailpho = pho2key(poo.typ_pho);
      if (!tailpho)
        pho_incr = FALSE;
    } else {
      if (!ggg.kval)
        pho_incr = FALSE;
    }
  }

  u_int64_t pp64[MAX_PHRASE_LEN + 1];
  phokey_t *pp = (phokey_t*)pp64;

  if (ph_key_sz==2) {
    extract_pho(chpho_idx, plen, pp);
  } else {
    extract_gtab_key(chpho_idx, plen, pp64);
  }


#if 0
  dbg("scanphr %d\n", plen);

  int t;
  for(t=0; t < plen; t++)
    prph(pp[t]);
  puts("");
#endif

  char pinyin_set[MAX_PH_BF_EXT];
  char *t_pinyin_set = NULL;
  gboolean is_pin_juyin = ph_key_sz==2 && pin_juyin;

  if (is_pin_juyin) {
    get_chpho_pinyin_set(pinyin_set);
    t_pinyin_set = pinyin_set + chpho_idx;
    mask_tone(pp, plen, t_pinyin_set);
  }

  int sti, edi;
  if (!tsin_seek(pp, plen, &sti, &edi, t_pinyin_set)) {
empty:
    if (rselN)
      *rselN = 0;
    return 0;
  }

  tss.pre_selN = 0;
  int maxlen=0;

#define selNMax 300
  PRE_SEL sel[selNMax];
  int selN = 0;


  u_int64_t mtk64[MAX_PHRASE_LEN+1];
  phokey_t *mtk = (phokey_t*) mtk64;
  u_int *mtk32 = (u_int *)mtk64;

  while (sti < edi && selN < selNMax) {
    u_char mtch[MAX_PHRASE_LEN*CH_SZ+1];
    char match_len;
    usecount_t usecount;

    load_tsin_entry(sti, &match_len, &usecount, mtk, mtch);

    sti++;
    if (plen > match_len || (pho_incr && plen==match_len)) {
      continue;
    }

    mask_tone(mtk, plen, t_pinyin_set);

    int i;
    for(i=0; i < plen; i++) {
      if (mtk[i]!=pp[i])
        break;
    }

    if (i < plen)
      continue;

    if (pho_incr) {
      if (ph_key_sz==2) {
        phokey_t last_m = mtk[plen];
        mask_key_typ_pho(&last_m);
        if (last_m != tailpho)
          continue;
      } else {
        u_int64_t v = ph_key_sz==4?mtk32[plen]:mtk64[plen];
        if (ggg.kval != (v&vmaskci))
          continue;
      }
    }


#if 0
    dbg("nnn ");
    nputs(mtch, match_len);
    dbg("\n");
#endif


    if (ph_key_sz==2) {
      if (check_fixed_mismatch(chpho_idx, (char *)mtch, plen))
        continue;
    } else {
      if (check_gtab_fixed_mismatch(chpho_idx, (char *)mtch, plen))
        continue;
    }

    if (maxlen < match_len)
      maxlen = match_len;

    sel[selN].len = match_len;
//    sel[selN].phidx = sti - 1;
    sel[selN].usecount = usecount;
    utf8cpyN(sel[selN].str, (char *)mtch, match_len);
    memcpy(sel[selN].phkey, mtk, match_len*ph_key_sz);
    selN++;
  }

//  dbg("SelN:%d\n", selN);

  if (selN > 1) {
    qsort(sel, selN, sizeof(PRE_SEL), qcmp_pre_sel_str);
    int nselN = 0;
    int i;
    for(i=0;i<selN;i++)
      if (sel[i].len>1 && (!i || strcmp(sel[i].str, sel[i-1].str)))
        sel[nselN++]=sel[i];
    selN = nselN;
  }

  if (selN==1 && sel[0].len<=2)
    goto empty;

  qsort(sel, selN, sizeof(PRE_SEL), qcmp_pre_sel_usecount);

//  dbg("selN:%d\n", selN);
  if (ph_key_sz==2)
    tss.pre_selN = Min(selN, phkbm.selkeyN);
  else
    tss.pre_selN = Min(selN, strlen(cur_inmd->selkey));

//  dbg("tss.pre_selN %d\n", tss.pre_selN);
  memcpy(tss.pre_sel, sel, sizeof(PRE_SEL) * tss.pre_selN);

  if (rselN)
    *rselN = selN;

  return maxlen;
}

void hide_pre_sel();
void chpho_get_str(int idx, int len, char *ch);
void disp_pre_sel_page();

void tsin_scan_pre_select(gboolean b_incr)
{
  if (!tsin_phrase_pre_select)
    return;
//  dbg("gtab_scan_pre_select %d\n", tss.c_len);

  tss.pre_selN = 0;

  hide_pre_sel();

  if (!tss.c_idx || !tss.c_len)
    return;

  init_pre_sel();

  int Maxlen = tss.c_len;
  if (Maxlen > MAX_PHRASE_LEN)
    Maxlen = MAX_PHRASE_LEN;

  int len, selN, max_len=-1, max_selN=-1;
  for(len=1; len <= Maxlen; len++) {
    int idx = tss.c_len - len;
    if (tss.chpho[idx].flag & FLAG_CHPHO_PHRASE_TAIL) {
//      dbg("phrase tail %d\n", idx);
      break;
    }
    int mlen = scanphr_e(tss.c_len - len, len, b_incr, &selN);
//	dbg("mlen %d len:%d\n", mlen, len);

    if (mlen) {
      max_len = len;
      max_selN = selN;
    }
  }

//  dbg("max_len:%d  max_selN:%d\n", max_len, max_selN);

  if (max_len < 0 || max_selN >= strlen(pho_selkey) * 2) {
    tss.pre_selN=0;
    return;
  }

  scanphr_e(tss.c_len - max_len, max_len, b_incr, &selN);

//  dbg("selN:%d %d\n", selN, tss.pre_selN);
  if (selN==1 && tss.pre_sel[0].len==max_len) {
    char out[MAX_PHRASE_LEN * CH_SZ + 1];
    chpho_get_str(tss.c_len - max_len, max_len, out);
    if (!strcmp(out, tss.pre_sel[0].str))
      return;
  }

//  dbg("selN %d %d\n",selN, tss.pre_selN);
  tss.ph_sta = tss.c_len - max_len;
  disp_pre_sel_page();
}
