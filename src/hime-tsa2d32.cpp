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

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "hime.h"
#include "pho.h"
#include "tsin.h"
#include "gtab.h"
#include "gst.h"

void load_pin_juyin();
phokey_t pinyin2phokey(char *s);

static char *bf;
//static int bfN_a = 0;
static gboolean b_pinyin;

int *phidx, *sidx, phcount;
int bfsize, phidxsize;
u_char *sf;
gboolean is_gtab, gtabkey64;
int phsz, hash_shift;
int (*key_cmp)(char *a, char *b, char len);

int key_cmp16(char *a, char *b, char len)
{
  u_char i;
  for(i=0; i < len; i++) {
    phokey_t ka,kb;
    memcpy(&ka, a, 2);
    memcpy(&kb, b, 2);
    if (ka > kb) return 1;
    if (kb > ka) return -1;
    a+=2;
    b+=2;
  }

  return 0;
}

int key_cmp32(char *a, char *b, char len)
{
  u_char i;
  for(i=0; i < len; i++) {
    u_int ka,kb;
    memcpy(&ka, a, 4);
    memcpy(&kb, b, 4);
    if (ka > kb) return 1;
    if (kb > ka) return -1;
    a+=4;
    b+=4;
  }
  return 0;
}

int key_cmp64(char *a, char *b, char len)
{
  u_char i;
  for(i=0; i < len; i++) {
    u_int64_t ka,kb;
    memcpy(&ka, a, 8);
    memcpy(&kb, b, 8);
    if (ka > kb) return 1;
    if (kb > ka) return -1;
    a+=8;
    b+=8;
  }
  return 0;
}

static int qcmp(const void *a, const void *b)
{
  int idxa=*((int *)a);  char *pa = (char *)&bf[idxa];
  int idxb=*((int *)b);  char *pb = (char *)&bf[idxb];
  u_char lena,lenb, len;
  usecount_t usecounta, usecountb;

  lena=*(pa++); memcpy(&usecounta, pa, sizeof(usecount_t)); pa+= sizeof(usecount_t);
  char *ka = pa;
  pa += lena * phsz;
  lenb=*(pb++); memcpy(&usecountb, pb, sizeof(usecount_t)); pb+= sizeof(usecount_t);
  char *kb = pb;
  pb += lenb * phsz;
  len=Min(lena,lenb);

  int d = (*key_cmp)(ka, kb, len);
  if (d)
    return d;

  if (lena > lenb)
    return 1;
  if (lena < lenb)
    return -1;

  int tlena = utf8_tlen(pa, lena);
  int tlenb = utf8_tlen(pb, lenb);

  if (tlena > tlenb)
    return 1;
  if (tlena < tlenb)
    return -1;

  if ((d=memcmp(pa, pb, tlena)))
    return d;

  // large first, so large one will be kept after delete
  return usecountb - usecounta;
}

static int qcmp_eq(const void *a, const void *b)
{
  int idxa=*((int *)a);  char *pa = (char *)&bf[idxa];
  int idxb=*((int *)b);  char *pb = (char *)&bf[idxb];
  u_char lena,lenb, len;

  lena=*(pa++);  pa+= sizeof(usecount_t);
  char *ka = pa;
  pa += lena * phsz;
  lenb=*(pb++);  pb+= sizeof(usecount_t);
  char *kb = pb;
  pb += lenb * phsz;
  len=Min(lena,lenb);

  int d = (*key_cmp)(ka, kb, len);
  if (d)
    return d;

  if (lena > lenb)
    return 1;
  if (lena < lenb)
    return -1;

  int tlena = utf8_tlen(pa, lena);
  int tlenb = utf8_tlen(pb, lenb);

  if (tlena > tlenb)
    return 1;
  if (tlena < tlenb)
    return -1;

  return memcmp(pa, pb, tlena);
}

static int qcmp_usecount(const void *a, const void *b)
{
  int idxa=*((int *)a);  char *pa = (char *)&sf[idxa];
  int idxb=*((int *)b);  char *pb = (char *)&sf[idxb];
  u_char lena,lenb, len;
  usecount_t usecounta, usecountb;

  lena=*(pa++); memcpy(&usecounta, pa, sizeof(usecount_t)); pa+= sizeof(usecount_t);
  lenb=*(pb++); memcpy(&usecountb, pb, sizeof(usecount_t)); pb+= sizeof(usecount_t);
  len=Min(lena,lenb);

  int d = (*key_cmp)(pa, pb, len);
  if (d)
    return d;
  pa += len*phsz;
  pb += len*phsz;

  if (lena > lenb)
    return 1;
  if (lena < lenb)
    return -1;

  // now lena == lenb
  int tlena = utf8_tlen(pa, lena);
  int tlenb = utf8_tlen(pb, lenb);

  if (tlena > tlenb)
    return 1;
  if (tlena < tlenb)
    return -1;

  return usecountb - usecounta;
}

void send_hime_message(Display *dpy, char *s);
#if WIN32 && 1
 #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

void init_TableDir();

int main(int argc, char **argv)
{
  FILE *fp,*fw;
  char s[1024];
  u_char chbuf[MAX_PHRASE_LEN * CH_SZ];
  u_short phbuf[80];
  u_int phbuf32[80];
  u_int64_t phbuf64[80];
  int i,j,idx,len, ofs;
  u_short kk;
  u_int64_t kk64;
  int hashidx[TSIN_HASH_N];
  u_char clen;
  int lineCnt=0;
  gboolean reload = getenv("HIME_NO_RELOAD")==NULL;

  if (reload) {
    dbg("need reload\n");
  } else {
    dbg("NO_GTK_INIT\n");
  }

  if (getenv("NO_GTK_INIT")==NULL)
    gtk_init(&argc, &argv);

  dbg("enter %s\n", argv[0]);

  if (argc < 2)
    p_err("must specify input file");


  init_TableDir();

  if ((fp=fopen(argv[1], "rb"))==NULL) {
     printf("Cannot open %s\n", argv[1]);
     exit(-1);
  }

  skip_utf8_sigature(fp);
  char *outfile;
  int fofs = ftell(fp);
  myfgets(s, sizeof(s), fp);
  if (strstr(s, "!!pinyin")) {
    b_pinyin = TRUE;
    printf("is pinyin\n");
    load_pin_juyin();
  } else
    fseek(fp, fofs, SEEK_SET);

  fofs = ftell(fp);
  int keybits=0, maxkey=0;
  char keymap[128];
  char kno[128];
  bzero(kno, sizeof(kno));
  myfgets(s, sizeof(s), fp);
  puts(s);
  if (strstr(s, TSIN_GTAB_KEY)) {
    is_gtab = TRUE;
    lineCnt++;

    if (argc < 3)
      p_err("useage %s input_file output_file", argv[0]);

    outfile = argv[2];

    len=strlen((char *)s);
    if (s[len-1]=='\n')
      s[--len]=0;
    char aa[128];
    keymap[0]=' ';
    sscanf(s, "%s %d %d %s", aa, &keybits, &maxkey, keymap+1);
    for(i=0; keymap[i]; i++)
      kno[keymap[i]]=i;

    if (maxkey * keybits > 32)
      gtabkey64 = TRUE;
  } else {
    if (argc==3)
      outfile = argv[2];
    else
      outfile = "tsin32";

    fseek(fp, fofs, SEEK_SET);
  }



  INMD inmd, *cur_inmd = &inmd;

  char *cphbuf;
  if (is_gtab) {
    cur_inmd->keybits = keybits;
    if (gtabkey64) {
      cphbuf = (char *)phbuf64;
      phsz = 8;
      key_cmp = key_cmp64;
      hash_shift = TSIN_HASH_SHIFT_64;
      cur_inmd->key64 = TRUE;
    } else {
      cphbuf = (char *)phbuf32;
      phsz = 4;
      hash_shift = TSIN_HASH_SHIFT_32;
      key_cmp = key_cmp32;
      cur_inmd->key64 = FALSE;
    }
    cur_inmd->last_k_bitn = (((cur_inmd->key64 ? 64:32) / cur_inmd->keybits) - 1) * cur_inmd->keybits;
    dbg("cur_inmd->last_k_bitn %d\n", cur_inmd->last_k_bitn);
  } else {
      cphbuf = (char *)phbuf;
      phsz = 2;
      key_cmp = key_cmp16;
      hash_shift = TSIN_HASH_SHIFT;
  }

  dbg("phsz: %d\n", phsz);

  phcount=ofs=0;
  while (!feof(fp)) {
    usecount_t usecount=0;

    lineCnt++;

    myfgets((char *)s,sizeof(s),fp);
    len=strlen((char *)s);
    if (s[0]=='#')
      continue;

    if (strstr(s, TSIN_GTAB_KEY))
      continue;

    if (s[len-1]=='\n')
      s[--len]=0;

    if (len==0)
      continue;

    i=0;
    int chbufN=0;
    int charN = 0;
    while (s[i]!=' ' && i<len) {
      int len = utf8_sz((char *)&s[i]);

      memcpy(&chbuf[chbufN], &s[i], len);

      i+=len;
      chbufN+=len;
      charN++;
    }

    while (i < len && s[i]==' ' || s[i]=='\t')
      i++;

    int phbufN=0;
    while (i<len && phbufN < charN && s[i]!=' ') {
      if (is_gtab) {
        kk64=0;
        int idx=0;
        while (s[i]!=' ' && i<len) {
          int k = kno[s[i]];
          kk64|=(u_int64_t)k << ( LAST_K_bitN - idx*keybits);
          i++;
          idx++;
        }

        if (phsz==8)
          phbuf64[phbufN++]=kk64;
        else
          phbuf32[phbufN++]=(u_int)kk64;
      } else {
        kk=0;
        if (b_pinyin) {
          kk = pinyin2phokey(s+i);
          while (s[i]!=' ' && i<len)
            i++;
        } else {
          while (s[i]!=' ' && i<len) {
            if (kk==(BACK_QUOTE_NO << 9))
              kk|=s[i];
            else
              kk |= lookup((u_char *)&s[i]);

            i+=utf8_sz((char *)&s[i]);
          }
        }

        phbuf[phbufN++]=kk;
      }

      i++;
    }

    if (phbufN!=charN) {
      p_err("%s   Line %d problem in phbufN!=chbufN %d != %d\n", s, lineCnt, phbufN, chbufN);
    }

    clen=phbufN;

    while (i<len && s[i]==' ')
      i++;

    if (i==len)
      usecount = 0;
    else
      usecount = atoi((char *)&s[i]);

    /*      printf("len:%d\n", clen); */

    if (phcount >= phidxsize) {
      phidxsize+=1024;
      if (!(phidx=(int *)realloc(phidx,phidxsize*4))) {
        puts("realloc err");
        exit(1);
      }
    }

    phidx[phcount++]=ofs;

    int new_bfN = ofs + 1 + sizeof(usecount_t)+ phsz * clen + chbufN;

    if (bfsize < new_bfN) {
      bfsize = new_bfN + 1024*1024;
      bf = (char *)realloc(bf, bfsize);
    }

    memcpy(&bf[ofs++],&clen,1);
    memcpy(&bf[ofs],&usecount, sizeof(usecount_t)); ofs+=sizeof(usecount_t);

    memcpy(&bf[ofs], cphbuf, clen * phsz);
    ofs+=clen * phsz;

    memcpy(&bf[ofs], chbuf, chbufN);
    ofs+=chbufN;
  }
  fclose(fp);

  /* dumpbf(bf,phidx); */

  puts("Sorting ....");

  qsort(phidx,phcount, sizeof(phidx[0]),qcmp);

  if (!(sf=(u_char *)malloc(bfsize))) {
    puts("malloc err");
    exit(1);
  }

  if (!(sidx=(int *)malloc(phidxsize*sizeof(int)))) {
    puts("malloc err");
    exit(1);
  }


  // delete duplicate
  ofs=0;
  j=0;
  for(i=0;i<phcount;i++) {
    idx = phidx[i];
    sidx[j]=ofs;
    len=bf[idx];
    int tlen = utf8_tlen(&bf[idx + 1 + sizeof(usecount_t) + phsz*len], len);
    clen= phsz*len + tlen + 1 + sizeof(usecount_t);

    if (i && !qcmp_eq(&phidx[i-1], &phidx[i]))
      continue;

    memcpy(&sf[ofs], &bf[idx], clen);
    j++;
    ofs+=clen;
  }

  phcount=j;
#if 1
  puts("Sorting by usecount ....");
  qsort(sidx, phcount, 4, qcmp_usecount);
#endif

  for(i=0;i<256;i++)
    hashidx[i]=-1;

  for(i=0;i<phcount;i++) {
    idx=sidx[i];
    idx+= 1 + sizeof(usecount_t);
    int v;

    if (phsz==2) {
      phokey_t kk;
      memcpy(&kk, &sf[idx], phsz);
      v = kk >> TSIN_HASH_SHIFT;
    } else if (phsz==4) {
      u_int kk32;
      memcpy(&kk32, &sf[idx], phsz);
      v = kk32 >> TSIN_HASH_SHIFT_32;
    }
    else if (phsz==8) {
      u_int64_t kk64;
      memcpy(&kk64, &sf[idx], phsz);
      v = kk64 >> TSIN_HASH_SHIFT_64;
    }

    if (v >= TSIN_HASH_N)
      p_err("error found %d", v);

    if (hashidx[v] < 0) {
      hashidx[v]=i;
    }
  }

  if (hashidx[0]==-1)
    hashidx[0]=0;

  hashidx[TSIN_HASH_N-1]=phcount;
  for(i=TSIN_HASH_N-2;i>=0;i--) {
    if (hashidx[i]==-1)
      hashidx[i]=hashidx[i+1];
  }

  for(i=1; i< TSIN_HASH_N; i++) {
    if (hashidx[i]==-1)
      hashidx[i]=hashidx[i-1];
  }

  printf("Writing data %s %d\n", outfile, ofs);
  if ((fw=fopen(outfile,"wb"))==NULL) {
    p_err("create err %s", outfile);
  }

  if (is_gtab) {
    TSIN_GTAB_HEAD head;
    bzero(&head, sizeof(head));
    strcpy(head.signature, TSIN_GTAB_KEY);
    head.keybits = keybits;
    head.maxkey = maxkey;
    strcpy(head.keymap, keymap);
    fwrite(&head, sizeof(head), 1, fw);
  }

  fwrite(sf,1,ofs,fw);
  fclose(fw);

  char outfileidx[512];
  strcat(strcpy(outfileidx, outfile), ".idx");

  dbg("Writing data %s\n", outfileidx);
  if ((fw=fopen(outfileidx,"wb"))==NULL) {
    p_err("cannot create %s", outfileidx);
  }

  fwrite(&phcount,4,1,fw);
  fwrite(hashidx,1,sizeof(hashidx),fw);
  fwrite(sidx,4,phcount,fw);
  printf("%d phrases\n",phcount);

  fclose(fw);
  free(sf);
  free(bf);


  if (reload) {
    printf("reload....\n");
    send_hime_message(
#if UNIX
    GDK_DISPLAY(),
#endif
    RELOAD_TSIN_DB);
  }

  exit(0);
}
