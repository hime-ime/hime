/* Copyright (C) 1995-2011 Edward Liu, Hsin-Chu, Taiwan
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
char tt[1024];


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


void get_line()
{
  while (!feof(fr)) {
    bzero(tt, sizeof(tt));
	myfgets(tt, sizeof(tt), fr);
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

void cmd_arg(char **cmd, char **arg)
{
  char *t;

  get_line();
  char *s=tt;

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
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

int main(int argc, char **argv)
{
  int i;
  char fname[64];
  char fname_cin[64];
  char fname_tab[64];
  char *cmd, *arg;
  struct TableHead th;
  int KeyNum;
  char kname[128][CH_SZ];
  char keymap[128];
  int chno;
  gtab_idx1_t idx1[256];
  char def1[256];
  int quick_def;
  int *phridx=NULL, phr_cou=0;
  char *phrbuf = NULL;
  int prbf_cou=0;

  if (!getenv("NO_GTK_INIT"))
    gtk_init(&argc, &argv);

  printf("-- cin2gtab encoding UTF-8 --\n");
  printf("--- please use iconv -f big5 -t utf-8 if your file is in big5 encoding\n");

  if (argc<=1) {
          printf("Enter table file name [.cin] : ");
          scanf("%s", fname);
  } else strcpy(fname,argv[1]);


  if (!strcmp(fname, "-v") || !strcmp(fname, "--version")) {
    dbg("cin2gtab for hime " HIME_VERSION "\n");
    exit(0);
  }

  char *p;
  if((p=strstr(fname, ".cin")))
    *p = 0;

  strcpy(fname_cin,fname);
  strcpy(fname_tab,fname);
  strcat(fname_cin,".cin");
  strcat(fname_tab,".gtab");

  if ((fr=fopen(fname_cin,"rb"))==NULL)
          p_err("Cannot open %s\n", fname_cin);

  skip_utf8_sigature(fr);


  bzero(&th,sizeof(th));
  bzero(kno,sizeof(kno));
  bzero(keymap,sizeof(keymap));

  bzero(itar,sizeof(itar));
  bzero(itout,sizeof(itout));
  bzero(itar64,sizeof(itar64));
  bzero(itout64,sizeof(itout64));

  cmd_arg(&cmd, &arg);
  if (sequ(cmd, "%gen_inp")) {
    dbg("skip gen_inp\n");
    cmd_arg(&cmd, &arg);
  }

  if (!sequ(cmd,"%ename") || !(*arg) )
    p_err("%d:  %%ename english_name  expected", lineno);
  arg[15]=0;
//  strcpy(th.ename,arg);

  cmd_arg(&cmd, &arg);
  if (!(sequ(cmd,"%prompt") || sequ(cmd,"%cname")) || !(*arg) )
    p_err("%d:  %%prompt prompt_name  expected", lineno);
  strncpy(th.cname, arg, MAX_CNAME);
  dbg("cname %s\n", th.cname);

  cmd_arg(&cmd, &arg);
  if (!sequ(cmd,"%selkey") || !(*arg) )
    p_err("%d:  %%selkey select_key_list expected", lineno);


  if (strlen(arg) >= sizeof(th.selkey)) {
    memcpy(th.selkey, arg, sizeof(th.selkey));
    strcpy(th.selkey2, arg+sizeof(th.selkey));
    dbg("th.selkey2 %s\n", th.selkey2);
  } else
    strcpy(th.selkey,arg);

  cmd_arg(&cmd, &arg);
  if (!sequ(cmd,"%dupsel") || !(*arg) ) {
    if (th.selkey[sizeof(th.selkey)-1])
      th.M_DUP_SEL = sizeof(th.selkey) + strlen(th.selkey2);
    else
      th.M_DUP_SEL = strlen(th.selkey);
  }
  else {
    th.M_DUP_SEL=atoi(arg);
    cmd_arg(&cmd, &arg);
  }

  for(;;) {
    if (sequ(cmd,"%endkey")) {
      strcpy(th.endkey, arg);
      cmd_arg(&cmd, &arg);
    } else
    if (sequ(cmd,"%space_style")) {
      th.space_style = (GTAB_space_pressed_E)atoi(arg);
      cmd_arg(&cmd, &arg);
    } else
    if (sequ(cmd,"%keep_key_case")) {
      th.flag |= FLAG_KEEP_KEY_CASE;
      cmd_arg(&cmd, &arg);
    } else
    if (sequ(cmd,"%symbol_kbm")) {
      th.flag |= FLAG_GTAB_SYM_KBM;
      cmd_arg(&cmd, &arg);
    } else
    if (sequ(cmd,"%phase_auto_skip_endkey")) {
      th.flag |= FLAG_PHRASE_AUTO_SKIP_ENDKEY;
      cmd_arg(&cmd, &arg);
    } else
    if (sequ(cmd,"%flag_auto_select_by_phrase")) {
      dbg("flag_auto_select_by_phrase\n");
      th.flag |= FLAG_AUTO_SELECT_BY_PHRASE;
      cmd_arg(&cmd, &arg);
    } else
    if (sequ(cmd,"%flag_disp_partial_match")) {
      dbg("flag_disp_partial_match\n");
      th.flag |= FLAG_GTAB_DISP_PARTIAL_MATCH;
      cmd_arg(&cmd, &arg);
    } else
      break;
  }


  if (!sequ(cmd,"%keyname") || !sequ(arg,"begin")) {
    p_err("%d:  %%keyname begin   expected, instead of %s %s", lineno, cmd, arg);
  }

  for(KeyNum=0;;) {
    char k;

    cmd_arg(&cmd, &arg);
    if (sequ(cmd,"%keyname")) break;
    if (BITON(th.flag, FLAG_KEEP_KEY_CASE))
      k=cmd[0];
    else
      k=mtolower(cmd[0]);

    if (kno[(int)k])
      p_err("%d:  key %c is already used",lineno, k);

    kno[(int)k]=++KeyNum;
    keymap[KeyNum]=k;
    bchcpy(&kname[KeyNum][0], arg);
  }

  keymap[0]=kname[0][0]=kname[0][1]=' ';
  KeyNum++;
  th.KeyS=KeyNum;    /* include space */

  cmd_arg(&cmd, &arg);

  if (sequ(cmd,"%quick") && sequ(arg,"begin")) {
    dbg(".. quick keys defined\n");
    for(quick_def=0;;) {
      char k;

      cmd_arg(&cmd, &arg);
      if (sequ(cmd,"%quick")) break;
      k=kno[mtolower(cmd[0])]-1;

      int N = 0;
      char *p = arg;

      if (strlen(cmd)==1) {
        while (*p) {
          int len=u8cpy(th.qkeys.quick1[(int)k][N++], p);
          p+=len;
        }
      } else
      if (strlen(cmd)==2) {
        int k1=kno[mtolower(cmd[1])]-1;
        while (*p) {
          char tp[4];
          int len=u8cpy(tp, p);

          if (utf8_eq(tp,_(_L("□"))))
             tp[0]=0;

          u8cpy(th.qkeys.quick2[(int)k][(int)k1][N++], tp);
          p+=len;
        }
      } else
        p_err("%d:  %quick only 1&2 keys are allowed '%s'", lineno, cmd);

      quick_def++;
    }
  }

  long pos=ftell(fr);
  int olineno = lineno;
  gboolean key64 = FALSE;
  int max_key_len = 0;

  while (!feof(fr)) {
    int len;

    cmd_arg(&cmd,&arg);
    if (!cmd[0] || !arg[0])
      continue;

    if (!strcmp(cmd, "%chardef")) {
      if (!strcmp(arg, "end"))
        break;
      else
        continue;
    }

    len=strlen(cmd);

    if (max_key_len < len)
      max_key_len = len;
  }


  fseek(fr, pos, SEEK_SET);
  lineno=olineno;

  INMD inmd, *cur_inmd = &inmd;

  cur_inmd->key64 = key64;
  cur_inmd->tbl64 = itout64;
  cur_inmd->tbl = itout;

  if (KeyNum < 64)
    cur_inmd->keybits = 6;
  else
    cur_inmd->keybits = 7;

  if (cur_inmd->keybits * max_key_len > 32) {
   cur_inmd->key64 = key64 = TRUE;
  }

  if (key64)
    dbg("key64\n");

  printf("KeyNum:%d keybits:%d\n", KeyNum, cur_inmd->keybits);

  th.keybits = cur_inmd->keybits;
  cur_inmd->last_k_bitn = (((cur_inmd->key64 ? 64:32) / cur_inmd->keybits) - 1) * cur_inmd->keybits;


  puts("char def");
  chno=0;
  while (!feof(fr)) {
    int len;
    u_int64_t kk;
    int k;

    cmd_arg(&cmd, &arg);
    if (!cmd[0] || !arg[0])
      continue;

    if (!strcmp(cmd, "%chardef")) {
      if (!strcmp(arg, "end"))
        break;
      else
        continue;
    }

    len=strlen(cmd);

    if (len > th.MaxPress) {
      th.MaxPress=len;
    }

    if (len > 10)
      p_err("%d:  only <= 10 keys is allowed '%s'", lineno, cmd);

    kk=0;
    for(i=0;i<len;i++) {
      int key = BITON(th.flag, FLAG_KEEP_KEY_CASE) ?
        cmd[i] : mtolower(cmd[i]);

      k=kno[key];

      if (!k)
        p_err("%d: key undefined in keyname '%c'\n", lineno, cmd[i]);

      kk|=(u_int64_t)k << ( LAST_K_bitN - i*th.keybits);
    }

//    dbg("%s kk:%llx\n", cmd, kk);

    if (key64) {
      memcpy(&itar64[chno].key, &kk, 8);
      itar64[chno].oseq=chno;
    }
    else {
      u_int key32 = (u_int)kk;

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

  printf("MaxPress: %d\n", th.MaxPress);

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


  u_int64_t keymask = KEY_MASK;
  for(i=0; i<chno; i++) {
    u_int64_t key = CONVT2(cur_inmd, i);
    int kk = (int)((key>>LAST_K_bitN) & keymask);

    if (!def1[kk]) {
      idx1[kk]=(gtab_idx1_t)i;
      def1[kk]=1;
    }
  }

  idx1[KeyNum]=chno;
  for(i=KeyNum-1;i>0;i--)
    if (!def1[i]) idx1[i]=idx1[i+1];

  if ((fw=fopen(fname_tab,"wb"))==NULL) {
    p_err("Cannot create");
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
    phridx[phr_cou++]=prbf_cou;
    printf("phrase count:%d\n", phr_cou);

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

#if 0
  char bzip2[128];
  strcat(strcpy(bzip2, "bzip2 -f -k "), fname_tab);
  system(bzip2);
#endif

  return 0;
}
