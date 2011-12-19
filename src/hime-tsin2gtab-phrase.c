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
#include "gtab.h"
// #include "gtab-phrase-db.h"
#include "tsin.h"
#include "lang.h"

#define MAX_K (500000)

ITEM it[MAX_K];
ITEM64 it64[MAX_K];
gboolean key64;
extern gboolean is_chs;
int itN;

int qcmp_ch(const void *aa, const void *bb)
{
  return memcmp(((ITEM *)aa)->ch, ((ITEM *)bb)->ch, CH_SZ);
}

int qcmp_ch64(const void *aa, const void *bb)
{
  return memcmp(((ITEM64 *)aa)->ch, ((ITEM64 *)bb)->ch, CH_SZ);
}

ITEM *find_ch(char *s, int *N)
{
  ITEM t;

  bzero(t.ch, CH_SZ);
  u8cpy((char *)t.ch, s);

  ITEM *p = (ITEM *)bsearch(&t, it, itN, sizeof(ITEM), qcmp_ch);
  if (!p)
    return NULL;

  ITEM *q = p+1;

  while (p > it && !qcmp_ch(p-1, &t))
    p--;

  ITEM *end = it + itN;
  while (q < end && !qcmp_ch(q, &t))
    q++;

  *N = q - p;
  if (*N > 20)
    p_err("err");

  return p;
}

ITEM64 *find_ch64(char *s, int *N)
{
  ITEM64 t;

  bzero(t.ch, CH_SZ);
  u8cpy((char *)t.ch, s);

  ITEM64 *p = (ITEM64 *)bsearch(&t, it64, itN, sizeof(ITEM64), qcmp_ch64);
  if (!p)
    return NULL;

  ITEM64 *q = p+1;

  while (p > it64 && !qcmp_ch64(p-1, &t))
    p--;

  ITEM64 *end = it64 + itN;
  while (q < end && !qcmp_ch64(q, &t))
    q++;

  *N = q - p;
  if (*N > 20)
    p_err("err");

  return p;
}

typedef struct {
  ITEM *arr;
  int N;
} KKARR;

typedef struct {
  ITEM64 *arr;
  int N;
} KKARR64;


void get_keymap_str(u_int64_t k, char *keymap, int keybits, char tkey[]);

#if WIN32
void init_hime_program_files();
 #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

int main(int argc, char **argv)
{
  gtk_init(&argc, &argv);

#if 1
  if (argc != 3)
    p_err("%s a_file.gtab outfile", argv[0]);
#endif
#if 1
  char *infile = argv[1];
  char *outfile = argv[2];
#else
  char *infile = "data/ar30.gtab";
  char *outfile = "l";
#endif

  FILE *fr;
  if ((fr=fopen(infile, "rb"))==NULL)
      p_err("cannot err open %s", infile);

  FILE *fp_out;
  if ((fp_out=fopen(outfile,"w"))==NULL) {
    printf("Cannot open %s", outfile);
    exit(-1);
  }

  struct TableHead th;
  fread(&th,1, sizeof(th), fr);
#if NEED_SWAP
  swap_byte_4(&th.version);
  swap_byte_4(&th.flag);
  swap_byte_4(&th.space_style);
  swap_byte_4(&th.KeyS);
  swap_byte_4(&th.MaxPress);
  swap_byte_4(&th.M_DUP_SEL);
  swap_byte_4(&th.DefC);
  for(i=0; i <= KeyNum; i++)
    swap_byte_4(&idx1[i]);
#endif
  int KeyNum = th.KeyS;
  dbg("keys %d\n",KeyNum);

  if (!th.keybits)
    th.keybits = 6;
  dbg("keybits:%d  maxPress:%d\n", th.keybits, th.MaxPress);

  int max_keyN;
  if (th.MaxPress*th.keybits > 32) {
    max_keyN = 64 / th.keybits;
    key64 = TRUE;
    dbg("it's a 64-bit .gtab\n");
  } else {
    max_keyN = 32 / th.keybits;
    key64 = FALSE;
  }

  dbg("key64:%d\n", key64);

  char kname[128][CH_SZ];
  char keymap[128];
  gtab_idx1_t idx1[256];
  static char kno[128];

  itN = th.DefC;

  bzero(keymap, sizeof(keymap));
  fread(keymap, 1, th.KeyS, fr);
  fread(kname, CH_SZ, th.KeyS, fr);
  fread(idx1, sizeof(gtab_idx1_t), KeyNum+1, fr);

  int i;
  for(i=0; i < th.KeyS; i++) {
    kno[keymap[i]] = i;
  }

  fprintf(fp_out,TSIN_GTAB_KEY" %d %d %s\n", th.keybits, th.MaxPress, keymap+1);

  if (key64) {
    fread(it64, sizeof(ITEM64), th.DefC, fr);
    qsort(it64, th.DefC, sizeof(ITEM64), qcmp_ch64);
  }
  else {
    fread(it, sizeof(ITEM), th.DefC, fr);
    qsort(it, th.DefC, sizeof(ITEM), qcmp_ch);
  }

  itN = th.DefC;

//  dbg("itN:%d\n", itN);
#if 0
  for(i=0; i < itN; i++) {
    printf("\n%d ", i);
    utf8_putchar(it64[i].ch);
  }
#endif

  fclose(fr);

  char fname[128];
  get_hime_user_fname(tsin32_f, fname);

  FILE *fp;
  if ((fp=fopen(fname,"rb"))==NULL) {
    printf("Cannot open %s", fname);
    exit(-1);
  }


  while (!feof(fp)) {
    int i;
    phokey_t phbuf[MAX_PHRASE_LEN];
    u_char clen;
    usecount_t usecount;

    fread(&clen,1,1,fp);
    fread(&usecount, sizeof(usecount_t), 1,fp);
    fread(phbuf,sizeof(phokey_t), clen, fp);

    char str[MAX_PHRASE_LEN * CH_SZ + 1];
    int strN = 0;
    KKARR kk[MAX_PHRASE_LEN];
    KKARR64 kk64[MAX_PHRASE_LEN];
    gboolean has_err = FALSE;

    if (key64)
      bzero(kk64, sizeof(kk64));
    else
      bzero(kk, sizeof(kk));

//    dbg("clen %d\n", clen);
    for(i=0;i<clen;i++) {
      char ch[CH_SZ];

      int n = fread(ch, 1, 1, fp);
      if (n<=0)
        goto stop;

      int len=utf8_sz(ch);

      fread(&ch[1], 1, len-1, fp);
//      utf8_putchar(ch);

      if (key64) {
        if (!(kk64[i].arr = find_ch64(ch, &kk64[i].N)))
          has_err = TRUE;
      } else {
        if (!(kk[i].arr = find_ch(ch, &kk[i].N)))
          has_err = TRUE;
      }

      memcpy(str+strN, ch, len);
      strN+=len;
    }

    if (has_err) {
//      dbg("has_error\n");
      continue;
    }
#if 0
    for(i=0; i < clen; i++)
      printf("%d ", kk64[i].N);
    printf("\n");
#endif
    str[strN]=0;

    int permN;
    if (key64) {
      permN=kk64[0].N;
      for(i=1;i<clen;i++)
        permN *= kk64[i].N;
    }
    else {
      permN=kk[0].N;
      for(i=1;i<clen;i++)
        permN *= kk[i].N;
    }

    int z;
    for(z=0; z < permN; z++) {
      char vz[MAX_PHRASE_LEN];

      int tz = z;

      if (key64) {
        for(i=0; i < clen; i++) {
          vz[i] = tz % kk64[i].N;
          tz /= kk64[i].N;
        }
      } else {
        for(i=0; i < clen; i++) {
          vz[i] = tz % kk[i].N;
          tz /= kk[i].N;
        }
      }

      char kstr[512];
      kstr[0]=0;

      for(i=0;i<clen;i++) {
         char tkey[16];
         u_int64_t k=0;

         if (key64) {
           memcpy(&k, kk64[i].arr[vz[i]].key, 8);
         } else {
           u_int t;
           memcpy(&t, kk[i].arr[vz[i]].key, 4);
           k = t;
         }

         get_keymap_str(k, keymap, th.keybits, tkey);

         strcat(kstr, tkey);
         strcat(kstr, " ");
      }

      fprintf(fp_out,"%s %s%d\n", str, kstr, usecount);
    }
  }
stop:

  fclose(fp);
  fclose(fp_out);
  return 0;
}
