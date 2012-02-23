/* Copyright (C) 2010 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include <string.h>
#include "hime.h"
#include "pho.h"
#include "tsin.h"
#include "hime-conf.h"
#include <math.h>
#include "tsin-parse.h"
#include "gtab-buf.h"
#include "gst.h"

#define DBG (0)
extern gboolean tsin_is_gtab;
extern int ph_key_sz;
void add_cache(int start, int usecount, TSIN_PARSE *out, short match_phr_N, short no_match_ch_N, int tc_len);
void extract_gtab_key(int start, int len, void *out);
gboolean check_gtab_fixed_mismatch(int idx, char *mtch, int plen);
void mask_tone(phokey_t *pho, int plen, char *tone_mask);

static int tsin_parse_len;

void set_tsin_parse_len(int len)
{
  tsin_parse_len = len;
}

static char *c_pinyin_set;

int tsin_parse_recur(int start, TSIN_PARSE *out,
                     short *r_match_phr_N, short *r_no_match_ch_N)
{
  int plen;
  double bestscore = -1;
  int bestusecount = 0;
  *r_match_phr_N = 0;
  *r_no_match_ch_N = tsin_parse_len - start;


  for(plen=1; start + plen <= tsin_parse_len && plen <= MAX_PHRASE_LEN; plen++) {
#if DBG
    dbg("---- aa st:%d hh plen:%d ", start, plen);utf8_putchar(tss.chpho[start].ch); dbg("\n");
#endif
    if (plen > 1) {
      if (tsin_is_gtab) {
        if (gbuf[start+plen-1].flag & FLAG_CHPHO_PHRASE_USER_HEAD)
          break;
      } else
        if (tss.chpho[start+plen-1].flag & FLAG_CHPHO_PHRASE_USER_HEAD)
          break;
    }

    phokey_t pp[MAX_PHRASE_LEN + 1];
    u_int pp32[MAX_PHRASE_LEN + 1];
    u_int64_t pp64[MAX_PHRASE_LEN + 1];
    int sti, edi;
    TSIN_PARSE pbest[MAX_PH_BF_EXT+1];
#define MAXV 1000
    int maxusecount = 5-MAXV;
    int remlen;
    short match_phr_N=0, no_match_ch_N = plen;
    void *ppp;

    if (ph_key_sz==2)
      ppp=pp;
    else if (ph_key_sz==4)
      ppp=pp32;
    else
      ppp=pp64;

    bzero(pbest, sizeof(TSIN_PARSE) * tsin_parse_len);

    pbest[0].len = plen;
    pbest[0].start = start;
    int i, ofs;

    if (tsin_is_gtab)
      for(ofs=i=0; i < plen; i++)
        ofs += utf8cpy((char *)pbest[0].str + ofs, gbuf[start + i].ch);
    else
      for(ofs=i=0; i < plen; i++)
        ofs += utf8cpy((char *)pbest[0].str + ofs, tss.chpho[start + i].ch);

#if DBG
    dbg("st:%d hh plen:%d ", start, plen);utf8_putchar(tss.chpho[start].ch); dbg("\n");
#endif

    if (tsin_is_gtab)
      extract_gtab_key(start, plen, ppp);
    else {
      extract_pho(start, plen, (phokey_t *)ppp);
      if (c_pinyin_set)
        mask_tone(pp, plen, c_pinyin_set + start);
    }

#if DBG
    for(i=0; i < plen; i++) {
      prph(pp[i]); dbg("%d", c_pinyin_set[i+start]);
    }
    dbg("\n");
#endif

    char *pinyin_set = c_pinyin_set ? c_pinyin_set+start:NULL;
    if (!tsin_seek(ppp, plen, &sti, &edi, pinyin_set)) {
//      dbg("tsin_seek not found...\n");
      if (plen > 1)
        break;
      goto next;
    }

    phokey_t mtk[MAX_PHRASE_LEN];
    u_int mtk32[MAX_PHRASE_LEN];
    u_int64_t mtk64[MAX_PHRASE_LEN];
    void *pho;

    if (ph_key_sz==2)
      pho=mtk;
    else if (ph_key_sz==4)
      pho=mtk32;
    else
      pho=mtk64;

    for (;sti < edi; sti++) {
      char mtch[MAX_PHRASE_LEN*CH_SZ+1];
      char match_len;
      usecount_t usecount;

      load_tsin_entry(sti, &match_len, &usecount, pho, (u_char *)mtch);

      if (match_len < plen)
        continue;

      if (tsin_is_gtab) {
        if (check_gtab_fixed_mismatch(start, mtch, plen))
          continue;
      } else
      if (check_fixed_mismatch(start, mtch, plen))
        continue;

      if (usecount < 0)
        usecount = 0;

      int i;
      if (ph_key_sz==2) {
        if (c_pinyin_set) {
//          mask_tone(pp, plen, c_pinyin_set + start);
          mask_tone(mtk, plen, c_pinyin_set + start);
        }
        for(i=0;i < plen;i++)
          if (mtk[i]!=pp[i])
            break;
      } else if (ph_key_sz==4) {
        for(i=0;i < plen;i++)
          if (mtk32[i]!=pp32[i])
            break;
      } else {
        for(i=0;i < plen;i++)
          if (mtk64[i]!=pp64[i])
            break;
      }

      if (i < plen)
        continue;

      if (match_len > plen) {
        continue;
      }

      if (usecount <= maxusecount)
        continue;

      pbest[0].len = plen;
      maxusecount = usecount;
      utf8cpyN((char *)pbest[0].str, mtch, plen);
      pbest[0].flag |= FLAG_TSIN_PARSE_PHRASE;

      match_phr_N = 1;
      no_match_ch_N = 0;
#if DBG
      utf8_putcharn(mtch, plen);
      dbg("   plen %d usecount:%d  ", plen, usecount);
        utf8_putcharn(mtch, plen);
      dbg("\n");
#endif
    }


next:

#if 0
    if (!match_phr_N) {
      if (tsin_is_gtab) {
        if (!(gbuf[start].ch[0] & 0x80))
          no_match_ch_N = 0;
      } else
      if (!(tss.chpho[start].ch[0] & 0x80))
        no_match_ch_N = 0;
    }
#else
//	dbg("no_match_ch_N %d\n", no_match_ch_N);
#endif

    remlen = tsin_parse_len - (start + plen);


    if (remlen) {
      int next = start + plen;
      CACHE *pca;

      short smatch_phr_N, sno_match_ch_N;
      int uc;

      if ((pca = cache_lookup(next))) {
        uc = pca->usecount;
        smatch_phr_N = pca->match_phr_N;
        sno_match_ch_N = pca->no_match_ch_N;
        memcpy(&pbest[1], pca->best, (tsin_parse_len - next) * sizeof(TSIN_PARSE));
      } else {
        uc = tsin_parse_recur(next, &pbest[1], &smatch_phr_N, &sno_match_ch_N);
//        dbg("   gg %d\n", smatch_phr_N);
        add_cache(next, uc, &pbest[1], smatch_phr_N, sno_match_ch_N, tsin_parse_len);
      }

      match_phr_N += smatch_phr_N;
      no_match_ch_N += sno_match_ch_N;
      maxusecount += uc;
    }


    double score = log((double)maxusecount + MAXV) /
      (pow((double)match_phr_N, 10)+ 1.0E-6) / (pow((double)no_match_ch_N, 20) + 1.0E-6);

#if DBG
    dbg("st:%d plen:%d zz muse:%d ma:%d noma:%d  score:%.4e %.4e\n", start, plen,
        maxusecount, match_phr_N, no_match_ch_N, score, bestscore);
#endif
    if (score > bestscore) {
#if DBG
      dbg("is best org %.4e\n", bestscore);
#endif
      bestscore = score;
      memcpy(out, pbest, sizeof(TSIN_PARSE) * (tsin_parse_len - start));

#if DBG
      dbg("    str:%d  ", start);
      int i;
      for(i=0;  i < tsin_parse_len - start; i++) {
        utf8_putcharn((char *)out[i].str, out[i].len);
      }
      dbg("\n");
#endif

      bestusecount = maxusecount;
      *r_match_phr_N = match_phr_N;
      *r_no_match_ch_N = no_match_ch_N;
    }
  }

  if (bestusecount < 0)
    bestusecount = 0;

  return bestusecount;
}

void disp_ph_sta_idx(int idx);

void free_cache(), load_tsin_db();
void tsin_parse()
{
  TSIN_PARSE out[MAX_PH_BF_EXT+1];
  bzero(out, sizeof(out));

  int i, ofsi;

  if (tss.c_len <= 1)
    return;

  load_tsin_db();

  set_tsin_parse_len(tss.c_len);

  init_cache(tss.c_len);

  char pinyin_set[MAX_PH_BF_EXT];
  c_pinyin_set = pin_juyin?pinyin_set:NULL;
  get_chpho_pinyin_set(pinyin_set);

  short smatch_phr_N, sno_match_ch_N;
  tsin_parse_recur(0, out, &smatch_phr_N, &sno_match_ch_N);

#if 0
  puts("vvvvvvvvvvvvvvvv");
  for(i=0;  i < tss.c_len; i++) {
    printf("%d:", out[i].len);
    utf8_putcharn(out[i].str, out[i].len);
  }
  dbg("\n");
#endif

  for(i=0; i < tss.c_len; i++)
    tss.chpho[i].flag &= ~(FLAG_CHPHO_PHRASE_HEAD|FLAG_CHPHO_PHRASE_BODY);

  for(ofsi=i=0; out[i].len; i++) {
    int j, ofsj;
    int psta = ofsi;

    if (out[i].flag & FLAG_TSIN_PARSE_PHRASE)
        tss.chpho[ofsi].flag |= FLAG_CHPHO_PHRASE_HEAD;

    for(ofsj=j=0; j < out[i].len; j++) {
      ofsj += utf8cpy(tss.chpho[ofsi].cha, (char *)&out[i].str[ofsj]);
//      tss.chpho[ofsi].ch = tss.chpho[ofsi].cha;

      tss.chpho[ofsi].flag |= FLAG_CHPHO_PHRASE_BODY;
      if (out[i].flag & FLAG_TSIN_PARSE_PHRASE)
        tss.chpho[ofsi].psta = psta;

      ofsi++;
    }
  }

  int ph_sta_idx = tss.ph_sta;
  if (tss.chpho[tss.c_len-1].psta>=0 && tss.c_len - tss.chpho[tss.c_len-1].psta > 1) {
    ph_sta_idx = tss.chpho[tss.c_len-1].psta;
  }

#if 1
  disp_ph_sta_idx(ph_sta_idx);
#endif

#if 0
  for(i=0;i<tss.c_len;i++)
    utf8_putchar(tss.chpho[i].ch);
  puts("");
#endif

  free_cache();
}
