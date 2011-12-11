/* Copyright (C) 2006-2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

FILE *fr, *fw;
int lineno;


char *skip_spc(char *s)
{
  while ((*s==' ' || *s=='\t') && *s) s++;
   return s;
}

char *to_spc(char *s)
{
  while (*s!=' ' && *s!='\t' && *s) s++;
    return s;
}

void del_nl_spc(char *s)
{
  char *t;

  int len=strlen(s);
  if (!*s) return;

  t=s+len-1;

  while (*t=='\n' || *t==' ' || (*t=='\t' && t > s))
    t--;

  *(t+1)=0;
}


void get_line(char *tt)
{
  while (!feof(fr)) {
    myfgets((char *)tt, 512, fr);
    lineno++;

    int len=strlen(tt);
    if (tt[len-1]=='\n')
      tt[len-1] = 0;

    if (tt[0]=='#' || strlen(tt) < 3)
      continue;
    else
      break;
  }
}

void cmd_arg(char *s, char **cmd, char **arg)
{
  char *t;

  get_line(s);

  if (!*s) {
    *cmd=*arg=s;
    return;
  }

  s=skip_spc(s);
  t=to_spc(s);
  *cmd=s;
  if (!(*t)) {
    *arg=t;
    return;
  }

  *t=0;
  t++;

  t=skip_spc(t);
  del_nl_spc(t);

  char *p;
  if ((p=strchr(t, '\t')))
    *p = 0;

  *arg=t;
}

int sequ(char *s, char *t)
{
  return (!strcmp(s,t));
}

typedef struct {
  u_int key;
  u_char ch[CH_SZ];
  int oseq;
} ITEM2;

typedef struct {
  u_int64_t key;
  u_char ch[CH_SZ];
  int oseq;
} ITEM2_64;


#define MAX_K (500000)

ITEM2 itar[MAX_K];
ITEM2_64 itar64[MAX_K];

ITEM itout[MAX_K];
ITEM64 itout64[MAX_K];


int qcmp(const void *aa, const void *bb)
{
  ITEM2 *a = (ITEM2 *)aa, *b = (ITEM2 *) bb;

  if (a->key > b->key) return 1;
  if (a->key < b->key) return -1;

  return a->oseq - b->oseq;
}


int qcmp_64(const void *aa, const void *bb)
{
  ITEM2_64 *a = (ITEM2_64 *)aa, *b = (ITEM2_64 *) bb;

  if (a->key > b->key) return 1;
  if (a->key < b->key) return -1;

  return a->oseq - b->oseq;
}


#define mtolower(ch) (ch>='A'&&ch<='Z'?ch+0x20:ch)

static char kno[128];

#if WIN32
void init_hime_program_files();
 #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

int main(int argc, char **argv)
{
  int i;
  char tt[512];
  char *cmd, *arg;
  struct TableHead th;
  int KeyNum;
  char kname[128][CH_SZ];
  char keymap[128];
  int chno;
  gtab_idx1_t idx1[256];
  char def1[256];
  int *phridx=NULL, phr_cou=0;
  char *phrbuf = NULL;
  int prbf_cou=0;

  gtk_init(&argc, &argv);

  INMD tinmd, *inp = &tinmd, *cur_inmd = &tinmd;

  if (argc != 4) {
    dbg("\thime-gtab-merge for hime " HIME_VERSION "\n");
    p_err("%s input_file.gtab  phrase_file.append   final-output.gtab", argv[0]);
  }

  if ((fr=fopen(argv[1], "rb"))==NULL)
      p_err("cannot err open %s", argv[1]);

  gboolean key64=64;

  inp->tbl64 = itout64;
  inp->tbl = itout;

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
  KeyNum = th.KeyS;
  dbg("keys %d\n",KeyNum);

  if (!th.keybits)
    th.keybits = 6;
  inp->keybits = th.keybits;
  dbg("keybits:%d\n", th.keybits);

  if (th.MaxPress*th.keybits > 32) {
    inp->max_keyN = 64 / th.keybits;
    key64 = inp->key64 = TRUE;
    dbg("it's a 64-bit .gtab\n");
  } else {
    inp->max_keyN = 32 / th.keybits;
    key64 = inp->key64 = FALSE;
  }

  inp->last_k_bitn = (((inp->key64 ? 64:32) / inp->keybits) - 1) * inp->keybits;
  dbg("inp->key64:%d\n", inp->key64);

  u_int64_t keymask = KEY_MASK;

  fread(keymap, 1, th.KeyS, fr);
  fread(kname, CH_SZ, th.KeyS, fr);
  fread(idx1, sizeof(gtab_idx1_t), KeyNum+1, fr);

  for(i=0; i < th.KeyS; i++) {
    kno[keymap[i]] = i;
  }

  for(i=0; i < th.DefC; i++) {
    ITEM it;
    ITEM64 it64;

    if (key64) {
      fread(&it64, sizeof(ITEM64), 1, fr);
      itar64[i].oseq = i;
      memcpy(itar64[i].ch, it64.ch, sizeof(it64.ch));
      memcpy(&itar64[i].key, it64.key, sizeof(it64.key));
    }
    else {
      fread(&it, sizeof(ITEM), 1, fr);
      itar[i].oseq = i;
      memcpy(itar[i].ch, it.ch, sizeof(it.ch));
      memcpy(&itar[i].key, it.key, sizeof(it.key));
    }
  }

  chno = th.DefC;
  fread(&phr_cou, sizeof(int), 1, fr);


  if (phr_cou) {
    phridx = tmalloc(int, phr_cou+1);
    fread(phridx, sizeof(int), phr_cou, fr);
    phr_cou--;
    prbf_cou = phridx[phr_cou];
    phrbuf = (char *)malloc(prbf_cou);
    fread(phrbuf, 1, prbf_cou, fr);
  }

  fclose(fr);
  dbg("input phr_cou %d  DefC:%d  prbf_cou:%d\n", phr_cou, chno, prbf_cou);

  if ((fr=fopen(argv[2], "rb"))==NULL)
      p_err("cannot err open %s", argv[2]);

  skip_utf8_sigature(fr);

  puts("char def");
  while (!feof(fr)) {
    int len;
    u_int64_t kk;
    int k;

    cmd_arg(tt, (char **)&cmd, (char **)&arg);
    if (!cmd[0] || !arg[0])
      continue;
    if (cmd[0]=='%')
      continue;

    len=strlen(cmd);

    if (len > inp->max_keyN)
      p_err("%d:  only <= %d keys is allowed '%s'  %s", lineno, inp->max_keyN, cmd, tt);

    kk=0;
    for(i=0;i<len;i++) {
      int key =  BITON(th.flag, FLAG_KEEP_KEY_CASE) ?
        cmd[i] : mtolower(cmd[i]);

      k=kno[key];
      kk|=(u_int64_t)k << ( LAST_K_bitN - i*cur_inmd->keybits);
    }

    if (key64) {
      memcpy(&itar64[chno].key, &kk, 8);
      itar64[chno].oseq=chno;
    }
    else {
      u_int key32 = kk;

      memcpy(&itar[chno].key, &key32, 4);
      itar[chno].oseq=chno;
    }

    if ((len=strlen(arg)) <= CH_SZ && (arg[0] & 0x80)) {
      char out[CH_SZ+1];

      bzero(out, sizeof(out));
      memcpy(out, arg, len);

      if (key64)
        bchcpy(itar64[chno].ch, out);
      else
        bchcpy(itar[chno].ch, out);

    } else {
      if (key64) {
          itar64[chno].ch[0]=phr_cou>>16;
          itar64[chno].ch[1]=(phr_cou >> 8) & 0xff;
          itar64[chno].ch[2]=phr_cou&0xff;
      }
      else {
          itar[chno].ch[0]=phr_cou>>16;
          itar[chno].ch[1]=(phr_cou >> 8) & 0xff;
          itar[chno].ch[2]=phr_cou&0xff;
      }

      if (len > MAX_CIN_PHR)
        p_err("phrase too long: %s  max:%d bytes\n", arg, MAX_CIN_PHR);

      phridx = trealloc(phridx, int, phr_cou+1);
      phridx[phr_cou++]=prbf_cou;
      phrbuf = (char *)realloc(phrbuf, prbf_cou + len + 1);
      strcpy(&phrbuf[prbf_cou],arg);
//      printf("phrase:%d  len:%d'%s'\n", phr_cou, len, arg);
      prbf_cou+=len;
    }

    chno++;
  }
  fclose(fr);

#define _sort qsort

  th.DefC=chno;
  cur_inmd->DefChars = chno;

  if (key64)
    _sort(itar64,chno,sizeof(ITEM2_64),qcmp_64);
  else
    _sort(itar,chno,sizeof(ITEM2),qcmp);

  if (key64) {
    for(i=0;i<chno;i++)
      memcpy(&itout64[i],&itar64[i],sizeof(ITEM64));
  } else {
    for(i=0;i<chno;i++)
      memcpy(&itout[i],&itar[i],sizeof(ITEM));
  }


  bzero(def1,sizeof(def1));
  bzero(idx1,sizeof(idx1));


  for(i=0; i<chno; i++) {
    u_int64_t key = CONVT2(cur_inmd, i);
#if 0
    dbg("%d] %llx %d %d %d %d zzz\n", i, key,
     cur_inmd->tbl[i].ch[0], cur_inmd->tbl[i].ch[1],
     cur_inmd->tbl[i].ch[2], cur_inmd->tbl[i].ch[3]);
#endif
    int kk = (key>>LAST_K_bitN) & keymask;

    if (!def1[kk]) {
      idx1[kk]=(gtab_idx1_t)i;
      def1[kk]=1;
    }
  }

  idx1[KeyNum]=chno;
  for(i=KeyNum-1;i>0;i--)
    if (!def1[i]) idx1[i]=idx1[i+1];

  if ((fw=fopen(argv[3],"wb"))==NULL) {
    p_err("Cannot create %s", argv[3]);
  }

  printf("Defined Characters:%d\n", chno);

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
  fwrite(&th,1,sizeof(th),fw);
  fwrite(keymap, 1, KeyNum, fw);
  fwrite(kname, CH_SZ, KeyNum, fw);
  fwrite(idx1, sizeof(gtab_idx1_t), KeyNum+1, fw);

  if (key64) {
#if NEED_SWAP
    for(i=0; i < chno; i++) {
      swap_byte_8(&itout64[i].key);
    }
#endif
    fwrite(itout64, sizeof(ITEM64), chno, fw);
#if 0
    for(i=0; i < 100; i++)
      dbg("%d] %c%c%c\n", i, itout64[i].ch[0], itout64[i].ch[1], itout64[i].ch[2]);
#endif
  }
  else {
#if NEED_SWAP
    for(i=0; i < chno; i++) {
      swap_byte_4(&itout[i].key);
    }
#endif
    fwrite(itout, sizeof(ITEM), chno, fw);
  }

  if (phr_cou) {
    printf("phrase count:%d\n", phr_cou);
    phridx[phr_cou++]=prbf_cou;

    int ophr_cou = phr_cou;
#if NEED_SWAP
    for(i=0; i < phr_cou; i++)
      swap_byte_4(&phridx[i]);
    swap_byte_4(&phr_cou);
#endif
    fwrite(&phr_cou, sizeof(int), 1, fw);
    fwrite(phridx, sizeof(int), ophr_cou, fw);
    fwrite(phrbuf,1,prbf_cou,fw);
  }

  fclose(fw);

  return 0;
}
