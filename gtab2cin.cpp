/* Copyright (C) 2011 Huang, Kai-Chang (Solomon Huang) <kaichanh@gmail.com>
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
#include <stdarg.h>
#include <sys/types.h>
#if FREEBSD
#include <sys/param.h>
#include <sys/stat.h>
#endif
#include <string.h>
#include "hime.h"
#include "gtab.h"
#include "hime-endian.h"
#include "hime-version.h"


int futf8cpy_bytes(FILE *fp, char *s, int n)
{
  int tn=0;

  while (tn < n && *s) {
    int sz = utf8_sz(s);

    fwrite(s, 1, sz, fp);
    tn+=sz;
    s+=sz;
  }
  return tn;
}

int is_endkey(struct TableHead *th, char key)
{
  int i;
  if (key == ' ') return 1;
  for (i = 0; i < sizeof(th->endkey); i++)
    if (key == th->endkey[i]) return 1;
  return 0;
}

u_int convert_key32(unsigned char *key32)
{
  u_int key = 0;
  key |= *(key32);
  key |= *(key32+1) << 8;
  key |= *(key32+2) << 16;
  key |= *(key32+3) << 24;
  return key;
}

u_int64_t convert_key64(unsigned char *key64)
{
  u_int64_t key = 0;
  key |= (u_int64_t)*(key64);
  key |= (u_int64_t)*(key64+1) << 8;
  key |= (u_int64_t)*(key64+2) << 16;
  key |= (u_int64_t)*(key64+3) << 24;
  key |= (u_int64_t)*(key64+4) << 32;
  key |= (u_int64_t)*(key64+5) << 40;
  key |= (u_int64_t)*(key64+6) << 48;
  key |= (u_int64_t)*(key64+7) << 56;
  return key;
}

int main(int argc, char **argv)
{
  const char CIN_HEADER[] = "#\n# cin file created via gtab2cin\n#\n";
  FILE *fr, *fw;
  char fname[64];
  char fname_cin[64];
  char fname_tab[64];
  struct TableHead *th;
  char *kname;
  char *keymap;
  int quick_def = 0;
  char *gtabbuf = NULL;
  long gtablen = 0;
  int key_idx, key_idx2, i, key_seq;
  QUICK_KEYS qkeys;
  int *phridx;
  char *phrbuf;

  if (!getenv("NO_GTK_INIT"))
    gtk_init(&argc, &argv);

  if (argc<=1) {
    printf("Enter table file name [.gtab] : ");
    scanf("%s", fname);
  } else strcpy(fname,argv[1]);

  strcpy(fname_cin,fname);
  strcpy(fname_tab,fname);
  strcat(fname_cin,".cin");
  strcat(fname_tab,".gtab");

  if ((fr=fopen(fname_tab,"rb"))==NULL)
    p_err("Cannot open %s\n", fname_tab);

  fseek(fr, 0L, SEEK_END);
  gtablen = ftell(fr);
  rewind(fr);
  gtabbuf = malloc(gtablen);
  if (gtabbuf && (gtablen != fread(gtabbuf, 1, gtablen, fr) || gtablen <= 0)) {
    fclose(fr);
    p_err("Read %s fail\n", fname_tab);
  }
  fclose(fr);
  if ((fw=fopen(fname_cin,"wb"))==NULL) {
    free(gtabbuf);
    p_err("Cannot create");
  }

  th = (struct TableHead *)gtabbuf;
  fprintf(fw, "%s", CIN_HEADER);
  fprintf(fw, "%%gen_inp\n");
  fprintf(fw, "%%ename %s\n", fname);
  fprintf(fw, "%%cname %s\n", th->cname);
  fprintf(fw, "%%selkey ");
  if (th->selkey[sizeof(th->selkey)-1] == 0) {
    fprintf(fw, "%s\n", th->selkey);
  }
  else {
    fwrite(th->selkey, 1, sizeof(th->selkey), fw);
    fprintf(fw, "%s\n", th->selkey2);
  }
  fprintf(fw, "%%dupsel %d\n", th->M_DUP_SEL);
  if (strlen(th->endkey))
    fprintf(fw, "%%endkey %s\n", th->endkey);
  fprintf(fw, "%%space_style %d\n", th->space_style);
  if (th->flag & FLAG_KEEP_KEY_CASE)
    fprintf(fw, "%%keep_key_case\n");
  if (th->flag & FLAG_GTAB_SYM_KBM)
    fprintf(fw, "%%symbol_kbm\n");
  if (th->flag & FLAG_PHRASE_AUTO_SKIP_ENDKEY)
    fprintf(fw, "%%phase_auto_skip_endkey\n");
  if (th->flag & FLAG_AUTO_SELECT_BY_PHRASE)
    fprintf(fw, "%%flag_auto_select_by_phrase\n");
  if (th->flag & FLAG_GTAB_DISP_PARTIAL_MATCH)
    fprintf(fw, "%%flag_disp_partial_match\n");

  keymap = gtabbuf + sizeof(struct TableHead);
  kname = keymap + th->KeyS;
  fprintf(fw, "%%keyname begin\n");
  for (key_idx = 1; key_idx < th->KeyS; key_idx++) {
    //prevent leading #
    if (*(keymap + key_idx) == '#')
      fprintf(fw, "%c", ' ');
    fprintf(fw, "%c ", *(keymap + key_idx));
    futf8cpy_bytes(fw, (kname + CH_SZ*key_idx), CH_SZ);
    fprintf(fw, "\n");
  }
  fprintf(fw, "%%keyname end\n");

  /* check quick def */
  bzero(&qkeys,sizeof(qkeys));
  if (0 != memcmp(&qkeys, &th->qkeys, sizeof(qkeys)))
    quick_def = 1;
  if (quick_def) {
    fprintf(fw, "%%quick begin\n");
    for (key_idx = 0; key_idx < th->KeyS; key_idx++) {
      if (is_endkey(th, *(keymap + key_idx + 1)))
        continue;
      fprintf(fw, "%c ", *(keymap + key_idx + 1));
      for (i = 0; i < 10; i++)
        futf8cpy_bytes(fw, th->qkeys.quick1[key_idx][i], CH_SZ);
      fprintf(fw, "\n");
    }
    for (key_idx = 0; key_idx < th->KeyS; key_idx++) {
      for (key_idx2 = 0; key_idx2 < th->KeyS; key_idx2++) {
        if (is_endkey(th, *(keymap + key_idx + 1)))
          continue;
        if (is_endkey(th, *(keymap + key_idx2 + 1)))
          continue;
        fprintf(fw, "%c%c ", *(keymap + key_idx + 1), *(keymap + key_idx2 + 1));
        for (i = 0; i < 10; i++) {
          if (0 == futf8cpy_bytes(fw, th->qkeys.quick2[key_idx][key_idx2][i], CH_SZ))
            futf8cpy_bytes(fw, _(_L("□")), CH_SZ);
        }
        fprintf(fw, "\n");
      }
    }
    fprintf(fw, "%%quick end\n");
  }

  fprintf(fw, "%%chardef begin\n");

  /* older gtab */
  if (th->keybits == 0) {
    if (th->MaxPress <= 5)
      th->keybits = 6;
    else
      th->keybits = 7;
  }

  if (th->keybits * th->MaxPress <= 32) {
    ITEM *item = (ITEM *)(kname + (CH_SZ * th->KeyS) + (sizeof(gtab_idx1_t) * (th->KeyS + 1)));
    u_int key;
    u_int mask = (1 << th->keybits) - 1;
    phridx = (int *)(item + th->DefC);
    phrbuf = (char *)(phridx + *phridx + 1);
    for (i = 0; i < th->DefC; i++) {
      key = convert_key32((unsigned char *)item->key);
      for (key_seq = 0; key_seq < th->MaxPress; key_seq++) {
        key_idx =
          ((key >> (th->keybits * ((32 / th->keybits) - key_seq - 1))) & mask);
        /* prevent leading # */
        if (key_seq == 0 && (*(keymap + key_idx) == '#'))
          fprintf(fw, "%c", ' ');
        fprintf(fw, "%c", *(keymap + key_idx));
      }
      fprintf(fw, " ");
      if (item->ch[0] == 0) { /* assume total phrases is less than 65535 */
        /* phrases define */
        int idx = 0, phr_len;
        char phr_str[MAX_CIN_PHR + 1];
        idx |= item->ch[0] << 16;
        idx |= item->ch[1] << 8;
        idx |= item->ch[2];
        memset(phr_str, 0, MAX_CIN_PHR + 1);
        phr_len = *(phridx + idx + 2) - *(phridx + idx + 1);
        memcpy(phr_str, phrbuf + *(phridx + idx + 1), phr_len);
        fprintf(fw, "%s\n", phr_str);
      }
      else {
        /* characters define */
        futf8cpy_bytes(fw, (char *)item->ch, CH_SZ);
        fprintf(fw, "\n");
      }
      item++;
    }
  }
  else if (th->keybits * th->MaxPress <= 64) {
    ITEM64 *item = (ITEM64 *)(kname + (CH_SZ * th->KeyS) + (sizeof(gtab_idx1_t) * (th->KeyS + 1)));
    u_int64_t key;
    u_int mask = (1L << th->keybits) - 1;
    phridx = (int *)(item + th->DefC);
    phrbuf = (char *)(phridx + *phridx + 1);
    for (i = 0; i < th->DefC; i++) {
      key = convert_key64((unsigned char *)item->key);
      for (key_seq = 0; key_seq < th->MaxPress; key_seq++) {
        key_idx =
          ((key >> (th->keybits * ((64 / th->keybits) - key_seq - 1))) & mask);
        /* prevent leading # */
        if (key_seq == 0 && (*(keymap + key_idx) == '#'))
          fprintf(fw, "%c", ' ');
        fprintf(fw, "%c", *(keymap + key_idx));
      }
      fprintf(fw, " ");
      if (item->ch[0] == 0) { /* assume total phrases is less than 65535 */
        /* phrases define */
        int idx = 0, phr_len;
        char phr_str[MAX_CIN_PHR + 1];
        idx |= item->ch[0] << 16;
        idx |= item->ch[1] << 8;
        idx |= item->ch[2];
        memset(phr_str, 0, MAX_CIN_PHR + 1);
        phr_len = *(phridx + idx + 2) - *(phridx + idx + 1);
        memcpy(phr_str, phrbuf + *(phridx + idx + 1), phr_len);
        fprintf(fw, "%s\n", phr_str);
        printf("%s\n", phr_str);
      }
      else {
        /* characters define */
        futf8cpy_bytes(fw, (char *)item->ch, CH_SZ);
        fprintf(fw, "\n");
      }
      item++;
    }
  }
  else
    fprintf(fw,"# Unknown chardef\n");
  fprintf(fw, "%%chardef end\n");

  fprintf(fw, "#\n");
  fprintf(fw, "# Gtab version: %d\n", th->version);
  fprintf(fw, "# flags: %#x\n", th->flag);
  fprintf(fw, "# keybits: %d\n", th->keybits);
  fprintf(fw, "# MaxPress: %d\n", th->MaxPress);
  fprintf(fw, "# Defined Characters : %d\n", th->DefC);
  fprintf(fw, "#\n");
  free(gtabbuf);
  fclose(fw);
  printf("gtab2cin done\n");
  return 0;
}
