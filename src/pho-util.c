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
#include <sys/stat.h>
#include <stdlib.h>
#if WIN32
#include <io.h>
#endif


char phofname[128]="";
extern char *TableDir;
u_short idxnum_pho;
PHO_IDX *idx_pho;
int ch_pho_ofs;
PHO_ITEM *ch_pho;
int ch_phoN;
char *pho_phrase_area;
int pho_phrase_area_sz;
static char pho_normal_tab[]="pho.tab2";
static char pho_huge_tab[]="pho-huge.tab2";
static char s_pho_normal_tab[]="s-pho.tab2";
static char s_pho_huge_tab[]="s-pho-huge.tab2";
void update_table_file(char *name, int version);
extern gboolean is_chs;

void pho_load()
{
  char *pho_tab;

  if (is_chs) {
    pho_tab = phonetic_huge_tab ? s_pho_huge_tab:s_pho_normal_tab;
  } else
    pho_tab = phonetic_huge_tab ? pho_huge_tab:pho_normal_tab;

  if (!getenv("HIME_TABLE_DIR") && phonetic_char_dynamic_sequence) {
    get_hime_user_fname(pho_tab, phofname);
#if UNIX
    if (access(phofname, W_OK) < 0){
#else
    if (_access(phofname, 02) < 0){
#endif
      char sys_file[256], vv[256];

      get_sys_table_file_name(sys_file, pho_tab);
#if UNIX
      sprintf(vv,"cp %s %s\n", sys_file, phofname);
      system(vv);
#else
      CopyFileA(sys_file, phofname, FALSE);
#endif
    }
  } else {
    get_sys_table_file_name(pho_tab, phofname);
    dbg("use system's pho, no dynamic adj\n");
  }

  update_table_file(pho_tab, 4);

  FILE *fr;

  if ((fr=fopen(phofname,"rb"))==NULL)
    p_err("err %s\n", phofname);

  fread(&idxnum_pho,sizeof(u_short),1,fr);
  fread(&idxnum_pho,sizeof(u_short),1,fr);
  fread(&ch_phoN,sizeof(int),1,fr);
  fread(&pho_phrase_area_sz, sizeof(pho_phrase_area_sz), 1,fr);

  if (idx_pho)
    free(idx_pho);
  idx_pho = tmalloc(PHO_IDX, idxnum_pho + 1);
  fread(idx_pho, sizeof(PHO_IDX), idxnum_pho, fr);

  ch_pho_ofs = ftell(fr);

  if (ch_pho)
    free(ch_pho);

  if (!(ch_pho=tmalloc(PHO_ITEM, ch_phoN)))
    p_err("malloc error");

  fread(ch_pho,sizeof(PHO_ITEM), ch_phoN, fr);
//  dbg("ch_phoN:%d  %d\n", ch_phoN, idxnum_pho);
  if (pho_phrase_area) {
    free(pho_phrase_area);
    pho_phrase_area = NULL;
  }
  if (pho_phrase_area_sz) {
    pho_phrase_area = tmalloc(char, pho_phrase_area_sz);
    fread(pho_phrase_area, 1,pho_phrase_area_sz, fr);
#if 0
    dbg("pho_phrase loaded %d\n", pho_phrase_area_sz);
    int i;
    for(i=0; i <pho_phrase_area_sz; i+=strlen(pho_phrase_area+i)+1) {
      dbg("  %s\n", pho_phrase_area+i);
    }
#endif
  }

  fclose(fr);

  idx_pho[idxnum_pho].key=0xffff;
  idx_pho[idxnum_pho].start=ch_phoN;

#if 0
  int i;
  for(i=0; i <ch_phoN; i++) {
    char tt[5];

    utf8cpy(tt, ch_pho[i].ch);
    dbg("oooo %s\n", tt);
  }
#endif
}


char *pho_idx_str2(int idx, int *is_phrase)
{
  static char tt[CH_SZ+1];

  unsigned char *p = (u_char *)ch_pho[idx].ch;

  if (*p==PHO_PHRASE_ESCAPE) {
    p++;
    int ofs = (*p) | *(p+1)<<8 | *(p+2)<<16;

//    dbg("idx:%d ofs:%d %s\n", idx, ofs, pho_phrase_area+ofs);
    *is_phrase = TRUE;
    return pho_phrase_area+ofs;
  } else {
    *is_phrase = FALSE;
    utf8cpy(tt, (char *)p);
    return tt;
  }
}

char *pho_idx_str(int idx)
{
  int is_phrase;
  return pho_idx_str2(idx, &is_phrase);
}

void free_pho_mem()
{
  if (ch_pho)
    free(ch_pho);
}

typedef struct {
  phokey_t key;
  short count;
} PH_COUNT;

static int qcmp_pho_count(const void *aa, const void *bb)
{
  PH_COUNT *a = (PH_COUNT *)aa;
  PH_COUNT *b = (PH_COUNT *)bb;

  return b->count - a->count;
}


int utf8_pho_keys(char *utf8, phokey_t *phkeys)
{
  int i;
  int ofs=0;
  int phkeysN=0;
  PH_COUNT phcou[256];

  do {
    for(; ofs < ch_phoN; ofs++)
      if (utf8_eq(utf8, pho_idx_str(ofs)))
        break;

    if (ofs==ch_phoN)
      goto ret;

    for(i=0; i < idxnum_pho; i++) {
      if (idx_pho[i].start<= ofs && ofs < idx_pho[i+1].start) {
//        dbg("ofs:%d %d  %d %d\n", ofs, i, idx_pho[i].start, idx_pho[i+1].start);
        phcou[phkeysN].count = ch_pho[ofs].count;
        phcou[phkeysN++].key = idx_pho[i].key;
        break;
      }
    }

    ofs++;
  } while (ofs < ch_phoN);

ret:

#if 0
    utf8_putchar(utf8);
    dbg("n %d\n", phkeysN);
#endif
  qsort(phcou, phkeysN, sizeof(PH_COUNT), qcmp_pho_count);

  for(i=0; i < phkeysN; i++)
    phkeys[i] = phcou[i].key;

  return phkeysN;
}

char *phokey_to_str2(phokey_t kk, int last_number)
{
  u_int k1,k2,k3,k4;
  static char phchars[PHO_CHAR_LEN * 4 + 1];
  int phcharsN=0;

  phokey_t okk = kk;
  k4=(kk&7);
  kk>>=3;
  k3=(kk&15) * PHO_CHAR_LEN;
  kk>>=4;
  k2=(kk&3) * PHO_CHAR_LEN;
  kk>>=2;
  k1=(kk&31) * PHO_CHAR_LEN;

  if (k1==BACK_QUOTE_NO * PHO_CHAR_LEN) {
    strcpy(phchars, _(_L("、")));
    int len=strlen(phchars);
    phchars[len++]=okk & 0x7f;
    phchars[len]=0;
    return phchars;
  }

  if (k1) {
    phcharsN+=u8cpy(phchars, &pho_chars[0][k1]);
  }

  if (k2) {
    phcharsN+=u8cpy(&phchars[phcharsN], &pho_chars[1][k2]);
  }

  if (k3)  {
    phcharsN+=u8cpy(&phchars[phcharsN], &pho_chars[2][k3]);
  }

  if (k4) {
//    dbg("k4 %d\n", k4);
    if (last_number)
      phchars[phcharsN++] = k4 + '0';
    else
      phcharsN+=u8cpy(&phchars[phcharsN], &pho_chars[3][k4 * PHO_CHAR_LEN]);
  }

  phchars[phcharsN] = 0;

  return phchars;
}


char *phokey_to_str(phokey_t kk)
{
  return phokey_to_str2(kk, 0);
}

void str_to_all_phokey_chars(char *u8_str, char *out)
{
  out[0]=0;

  while (*u8_str) {
    phokey_t phos[32];

    int n=utf8_pho_keys(u8_str, phos);
#if 0
    utf8_putchar(u8_str);
    dbg("n %d\n", n);
#endif
    int i;
    for(i=0; i < n; i++) {
      char *pstr = phokey_to_str(phos[i]);
      strcat(out, pstr);
      if (i < n -1)
        strcat(out, " ");
    }

    u8_str+=utf8_sz(u8_str);

    if (*u8_str)
      strcat(out, " | ");
  }
}
