/* Copyright (C) 1995-2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
#include "tsin.h"
#include "gtab.h"
#include "gst.h"
#include "lang.h"

int hashidx[TSIN_HASH_N];
//static int *phidx;
static FILE *fp_phidx;
FILE *fph;
int phcount;
int ph_key_sz; // bytes
gboolean tsin_is_gtab;
static int tsin_hash_shift;

#define PHIDX_SKIP  (sizeof(phcount) + sizeof(hashidx))

char *current_tsin_fname;
int ts_gtabN;
//static int *ts_gtab_hash;
#define HASHN 256

static int a_phcount;

void get_hime_user_or_sys_fname(char *name, char fname[]);

void load_tsin_db0(char *infname, gboolean is_gtab_i)
{
  char tsidxfname[512];
//  dbg("cur %s %s\n", infname, current_tsin_fname);

  if (current_tsin_fname && !strcmp(current_tsin_fname, infname))
    return;

  strcpy(tsidxfname, infname);
  strcat(tsidxfname, ".idx");

//  dbg("tsidxfname %s\n", tsidxfname);

  FILE *fr;

  if ((fr=fopen(tsidxfname,"rb+"))==NULL) {
    p_err("load_tsin_db0 A Cannot open '%s'\n", tsidxfname);
  }


  fread(&phcount,4,1,fr);
#if     0
  printf("phcount:%d\n",phcount);
#endif
  a_phcount=phcount+256;
  fread(&hashidx,1,sizeof(hashidx),fr);

  fp_phidx = fr;

  if (fph)
    fclose(fph);

  dbg("tsfname: %s\n", infname);

  if ((fph=fopen(infname,"rb+"))==NULL)
    p_err("load_tsin_db0 B Cannot open '%s'", infname);

  free(current_tsin_fname);
  current_tsin_fname = strdup(infname);

  if (is_gtab_i) {
    TSIN_GTAB_HEAD head;
    fread(&head, sizeof(head), 1, fph);
    if (head.keybits*head.maxkey > 32) {
      ph_key_sz = 8;
      tsin_hash_shift = TSIN_HASH_SHIFT_64;
    }
    else {
      ph_key_sz = 4;
      tsin_hash_shift = TSIN_HASH_SHIFT_32;
    }
  } else {
    ph_key_sz = 2;
    tsin_hash_shift = TSIN_HASH_SHIFT;
  }
  tsin_is_gtab = is_gtab_i;
}




void free_tsin()
{
  free(current_tsin_fname); current_tsin_fname=NULL;

  if (fph) {
    fclose(fph); fph = NULL;
  }

  if (fp_phidx) {
    fclose(fp_phidx); fp_phidx=NULL;
  }
}

extern gboolean is_chs;
void load_tsin_db()
{
  char tsfname[512];
  char *fname = tsin32_f;

  get_hime_user_or_sys_fname(fname, tsfname);
  load_tsin_db0(tsfname, FALSE);
}

static void seek_fp_phidx(int i)
{
  fseek(fp_phidx, PHIDX_SKIP + i*sizeof(int), SEEK_SET);
}

void reload_tsin_db()
{
  char tt[512];
  if (!current_tsin_fname)
    return;

  strcpy(tt, current_tsin_fname);
  free(current_tsin_fname); current_tsin_fname = NULL;
  load_tsin_db0(tt, tsin_is_gtab);
}

inline static int get_phidx(int i)
{
  seek_fp_phidx(i);
  int t;
  fread(&t, sizeof(int), 1, fp_phidx);

  if (tsin_is_gtab)
    t += sizeof(TSIN_GTAB_HEAD);

  return t;
}


static inline int phokey_t_seq16(phokey_t *a, phokey_t *b, int len)
{
  int i;

  for (i=0;i<len;i++) {
    if (a[i] > b[i]) return 1;
    else
    if (a[i] < b[i]) return -1;
  }

  return 0;
}


static inline int phokey_t_seq32(u_int *a, u_int *b, int len)
{
  int i;

  for (i=0;i<len;i++) {
    if (a[i] > b[i]) return 1;
    else
    if (a[i] < b[i]) return -1;
  }

  return 0;
}


static inline int phokey_t_seq64(u_int64_t *a, u_int64_t *b, int len)
{
  int i;

  for (i=0;i<len;i++) {
    if (a[i] > b[i]) return 1;
    else
    if (a[i] < b[i]) return -1;
  }

  return 0;
}


static int phokey_t_seq(void *a, void *b, int len)
{
  if (ph_key_sz==2)
    return phokey_t_seq16((phokey_t *)a, (phokey_t *)b, len);
  if (ph_key_sz==4)
    return phokey_t_seq32((u_int *)a, (u_int *)b, len);
  if (ph_key_sz==8)
    return phokey_t_seq64((u_int64_t*)a, (u_int64_t*)b, len);
  return 0;
}


static int phseq(u_char *a, u_char *b)
{
  u_char lena, lenb, mlen;

  lena=*(a++); lenb=*(b++);
  a+=sizeof(usecount_t); b+=sizeof(usecount_t);   // skip usecount

  mlen=Min(lena,lenb);
  u_int64_t ka[MAX_PHRASE_LEN], kb[MAX_PHRASE_LEN];

  memcpy(ka, a, ph_key_sz * mlen);
  memcpy(kb, b, ph_key_sz * mlen);

  int d = phokey_t_seq(a, b, mlen);
  if (d)
    return d;

  if (lena > lenb) return 1;
  if (lena < lenb) return -1;
  return 0;
}

void inc_dec_tsin_use_count(void *pho, char *ch, int N);

static gboolean saved_phrase;

gboolean save_phrase_to_db(void *phkeys, char *utf8str, int len, usecount_t usecount)
{
  int mid, ord = 0, ph_ofs, hashno;
  u_char tbuf[MAX_PHRASE_LEN*(sizeof(u_int64_t)+CH_SZ) + 1 + sizeof(usecount_t)],
         sbuf[MAX_PHRASE_LEN*(sizeof(u_int64_t)+CH_SZ) + 1 + sizeof(usecount_t)];

  saved_phrase = TRUE;

  tbuf[0]=len;
  memcpy(&tbuf[1], &usecount, sizeof(usecount));  // usecount
  int tlen = utf8_tlen(utf8str, len);
#if 0
  dbg("tlen %d  '", tlen);
  for(i=0; i < tlen; i++)
    putchar(utf8str[i]);
  dbg("'\n");
#endif

  dbg("save_phrase_to_db '%s'  tlen:%d\n", utf8str, tlen);

  memcpy(&tbuf[1 + sizeof(usecount_t)], phkeys, ph_key_sz * len);
  memcpy(&tbuf[ph_key_sz*len + 1 + sizeof(usecount_t)], utf8str, tlen);

  if (ph_key_sz==2)
    hashno= *((phokey_t *)phkeys) >> TSIN_HASH_SHIFT;
  else if (ph_key_sz==4)
    hashno= *((u_int *)phkeys) >> TSIN_HASH_SHIFT_32;
  else
    hashno= *((u_int64_t *)phkeys) >> TSIN_HASH_SHIFT_64;

//  dbg("hashno %d\n", hashno);

  if (hashno >= TSIN_HASH_N)
    return FALSE;

  for(mid=hashidx[hashno]; mid<hashidx[hashno+1]; mid++) {
    ph_ofs=get_phidx(mid);

    fseek(fph, ph_ofs, SEEK_SET);
    fread(sbuf,1,1,fph);
    fread(&sbuf[1], sizeof(usecount_t), 1, fph); // use count
    fread(&sbuf[1+sizeof(usecount_t)], 1, (ph_key_sz + CH_SZ) * sbuf[0], fph);
    if ((ord=phseq(sbuf,tbuf)) > 0)
      break;

    if (!ord && !memcmp(&sbuf[sbuf[0]*ph_key_sz+1+sizeof(usecount_t)], utf8str, tlen)) {
//    bell();
      dbg("Phrase already exists\n");
      inc_dec_tsin_use_count(phkeys, utf8str, len);
      return FALSE;
    }
  }

  int wN = phcount - mid;

//  dbg("wN %d  phcount:%d mid:%d\n", wN, phcount, mid);

  if (wN > 0) {
    int *phidx = tmalloc(int, wN);
    seek_fp_phidx(mid);
    fread(phidx, sizeof(int), wN, fp_phidx);
    seek_fp_phidx(mid+1);
    fwrite(phidx, sizeof(int), wN, fp_phidx);
    free(phidx);
  }

  fseek(fph,0,SEEK_END);

  ph_ofs=ftell(fph);
  if (tsin_is_gtab)
    ph_ofs -= sizeof(TSIN_GTAB_HEAD);

//  dbg("ph_ofs %d  ph_key_sz:%d\n", ph_ofs, ph_key_sz);
  seek_fp_phidx(mid);
  fwrite(&ph_ofs, sizeof(int), 1, fp_phidx);
  phcount++;

  fwrite(tbuf, 1, ph_key_sz*len + tlen + 1+ sizeof(usecount_t), fph);
  fflush(fph);

  if (hashidx[hashno]>mid)
    hashidx[hashno]=mid;

  for(hashno++; hashno<TSIN_HASH_N; hashno++)
    hashidx[hashno]++;

  rewind(fp_phidx);
  fwrite(&phcount, sizeof(phcount), 1, fp_phidx);
  fwrite(&hashidx,sizeof(hashidx),1, fp_phidx);
  fflush(fp_phidx);

//  dbg("ofs %d\n", get_phidx(mid));

  return TRUE;
}


#include <sys/stat.h>


void load_tsin_entry0(char *len, usecount_t *usecount, void *pho, u_char *ch)
{
  *usecount = 0;
  *len = 0;
  fread(len, 1, 1, fph);

  if (*len > MAX_PHRASE_LEN || *len <= 0) {
    dbg("err: tsin db changed reload\n");
    reload_tsin_db(); // probably db changed, reload;
    *len = 0;
    return;
  }

  fread(usecount, sizeof(usecount_t), 1, fph); // use count
  fread(pho, ph_key_sz, (int)(*len), fph);
  if (ch) {
    fread(ch, CH_SZ, (int)(*len), fph);
    int tlen = utf8_tlen((char *)ch, *len);
    ch[tlen]=0;
  }
}


void load_tsin_entry(int idx, char *len, usecount_t *usecount, void *pho, u_char *ch)
{
  *usecount = 0;


  if (idx >= phcount) {
    reload_tsin_db(); // probably db changed, reload;
    *len = 0;
    return;
  }

  int ph_ofs=get_phidx(idx);
//  dbg("idx %d:%d\n", idx, ph_ofs);

  fseek(fph, ph_ofs , SEEK_SET);
  load_tsin_entry0(len, usecount, pho, ch);
}

// tone_mask : 1 -> pho has tone
void mask_tone(phokey_t *pho, int plen, char *tone_mask)
{
  int i;
//  dbg("mask_tone\n");
  if (!tone_mask)
    return;

  for(i=0; i < plen; i++) {
   if (!tone_mask[i])
    pho[i] &= (~7);
  }
}


// ***  r_sti<=  range  < r_edi
gboolean tsin_seek(void *pho, int plen, int *r_sti, int *r_edi, char *tone_mask)
{
  int mid, cmp;
  u_int64_t ss[MAX_PHRASE_LEN], stk[MAX_PHRASE_LEN];
  char len;
  usecount_t usecount;
  int hashi;

#if 0
  dbg("tsin_seek %d\n", plen);
#endif

#if 0
  if (tone_mask)
    mask_tone((phokey_t *)pho, plen, tone_mask);
#endif

  if (ph_key_sz==2)
    hashi= *((phokey_t *)pho) >> TSIN_HASH_SHIFT;
  else if (ph_key_sz==4)
    hashi= *((u_int *)pho) >> TSIN_HASH_SHIFT_32;
  else
    hashi= *((u_int64_t *)pho) >> TSIN_HASH_SHIFT_64;

  if (hashi >= TSIN_HASH_N) {
//    dbg("hashi >= TSIN_HASH_N\n");
    return FALSE;
  }

  int top=hashidx[hashi];
  int bot=hashidx[hashi+1];

  if (top>=phcount) {
//    dbg("top>=phcount\n");
    return FALSE;
  }

  while (top <= bot) {
    mid=(top+bot)/ 2;
    load_tsin_entry(mid, &len, &usecount, ss, NULL);

    u_char mlen;
    if (len > plen)
      mlen=plen;
    else
      mlen=len;

//    prphs(ss, mlen);
//    mask_tone((phokey_t *)ss, mlen, tone_mask);

#if DBG || 0
    int j;
    dbg("> ");
    prphs(ss, len);
    dbg("\n");
#endif

    cmp=phokey_t_seq(ss, pho, mlen);

    if (!cmp && len < plen)
      cmp=-2;

    if (cmp>0)
      bot=mid-1;
    else
    if (cmp<0)
      top=mid+1;
    else
      break;
  }

  if (cmp && !tone_mask) {
//    dbg("no match %d\n", cmp);
    return FALSE;
  }

//  dbg("<--\n");
  // seek to the first match because binary search is used
  gboolean found=FALSE;

  int sti;
  for(sti = mid; sti>=0; sti--) {
    load_tsin_entry(sti, &len, &usecount, stk, NULL);

    u_char mlen;
    if (len > plen)
      mlen=plen;
    else
      mlen=len;
#if 0
    prphs(stk, len);
#endif
    mask_tone((phokey_t *)stk, mlen, tone_mask);

    int v = phokey_t_seq(stk, pho, plen);
    if (!v)
      found = TRUE;

#if 0
    int j;
    dbg("%d] %d*> ", sti, mlen);
    prphs(stk, len);
    dbg(" v:%d\n", v);
#endif

    if ((!tone_mask && !v && len>=plen) ||
        (tone_mask && v>0 || !v && len >= plen))
      continue;
    break;
  }
  sti++;

  // seek to the tail

  if (tone_mask) {
    int top=hashidx[hashi];
    int bot=hashidx[hashi+1];

    if (top>=phcount) {
  //    dbg("top>=phcount\n");
      return FALSE;
    }

    phokey_t tpho[MAX_PHRASE_LEN];

    int i;
    for(i=0; i < plen; i++)
      tpho[i]=((phokey_t*)pho)[i] | 7;

    while (top <= bot) {
      mid=(top+bot)/ 2;
      load_tsin_entry(mid, &len, &usecount, ss, NULL);

      u_char mlen;
      if (len > plen)
        mlen=plen;
      else
        mlen=len;

  //    prphs(ss, mlen);

#if DBG || 0
      int j;
      dbg("> ");
      prphs(ss, len);
      dbg("\n");
#endif

      cmp=phokey_t_seq(ss, tpho, mlen);

      if (!cmp && len < plen)
        cmp=-2;

      if (cmp>0)
        bot=mid-1;
      else
      if (cmp<0)
        top=mid+1;
      else
        break;
    }
  }

  int edi;
  for(edi = mid; edi < phcount; edi++) {
    load_tsin_entry(edi, &len, &usecount, stk, NULL);

    u_char mlen;
    if (len > plen)
      mlen=plen;
    else
      mlen=len;
#if 0
    prphs(stk, len);
#endif
    mask_tone((phokey_t *)stk, mlen, tone_mask);

    int v = phokey_t_seq(stk, pho, plen);
    if (!v)
      found = TRUE;
#if 0
    dbg("edi%d -> ", edi);
    prphs(stk, len);
    dbg(" v:%d\n", v);
#endif

    if ((!tone_mask && !v && len >= plen)
      || (tone_mask && v<0 || !v && len >= plen))
      continue;
    break;
  }

#if 0
  dbg("sti%d edi:%d found:%d\n", sti, edi, found);
#endif

  *r_sti = sti;
  *r_edi = edi;

  return edi > sti;
}

void inc_dec_tsin_use_count(void *pho, char *ch, int N)
{
  int sti, edi;

//  dbg("inc_dec_tsin_use_count '%s'\n", ch);

  if (!tsin_seek(pho, N, &sti, &edi, NULL))
    return;

  int idx;

#if 0
  int tlen = strlen(ch);

  dbg("otlen %d  ", tlen);
  int i;
  for(i=0; i < tlen; i++)
    putchar(ch[i]);
  puts("");
#endif

  for(idx=sti; idx < edi; idx++) {
    char len;
    usecount_t usecount, n_usecount;
    u_int64_t phi[MAX_PHRASE_LEN];
    char stch[MAX_PHRASE_LEN * CH_SZ * 2];

    load_tsin_entry(idx, &len, &usecount, phi, (u_char *)stch);
    n_usecount = usecount;

    if (len!=N || phokey_t_seq(phi, pho, N))
      break;
#if 0
    for(i=0; i < tlen; i++)
      putchar(stch[i]);
    dbg(" ppp\n");
#endif

//	dbg("stch %s\n", stch);
    if (strcmp(stch, ch))
      continue;
#if 0
    dbg("found match\n");
#endif
    int ph_ofs=get_phidx(idx);

    fseek(fph, ph_ofs + 1, SEEK_SET);

    if (usecount < 0x3fffffff)
      n_usecount++;

    if (n_usecount != usecount) {
      fwrite(&n_usecount, sizeof(usecount_t), 1, fph); // use count
      fflush(fph);
    }
  }
}
