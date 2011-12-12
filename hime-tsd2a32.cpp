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
#include "tsin.h"

int phcount;
void prph2(FILE *fp, phokey_t kk);

#if WIN32
void init_hime_program_files();
 #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

void get_keymap_str(u_int64_t k, char *keymap, int keybits, char tkey[]);
char *phokey2pinyin(phokey_t k);
gboolean is_pinyin_kbm();
char *sys_err_strA();
void init_TableDir();

int main(int argc, char **argv)
{
  FILE *fp;
  int i;
  char clen;
  usecount_t usecount;
  gboolean pr_usecount = TRUE;
  char *fname;
  char *fname_out = NULL;

  gtk_init(&argc, &argv);

  if (argc <= 1) {
    printf("%s: file name expected\n", argv[0]);
    exit(1);
  }

  init_TableDir();

  gboolean b_pinyin = is_pinyin_kbm();

  for(i=1; i < argc;) {
    if (!strcmp(argv[i], "-nousecount")) {
      i++;
      pr_usecount = FALSE;
      b_pinyin = FALSE;
    } else
    if (!strcmp(argv[i], "-o")) {
      if (i==argc-1)
        p_err("-o need out file name");
        fname_out = argv[i+1];
        i+=2;
    } else
      fname = argv[i++];
  }

  FILE *fp_out;

  if (!fname_out) {
    fp_out = stdout;
  } else {
    dbg("%s use %s\n", argv[0], fname_out);
    fp_out = fopen(fname_out, "w");
    if (!fp_out)
      p_err("cannot create %s\n", fname_out);

  }

  if (b_pinyin)
    fprintf(fp_out, "!!pinyin\n");

  if ((fp=fopen(fname,"rb"))==NULL)
    p_err("Cannot open %s %s", fname, sys_err_strA());


  TSIN_GTAB_HEAD head;
  int phsz = 2;

  fread(&head, sizeof(head), 1, fp);
  if (!strcmp(head.signature, TSIN_GTAB_KEY)) {
    if (head.maxkey * head.keybits > 32)
      phsz = 8;
    else
      phsz = 4;
  } else
    rewind(fp);

  if (phsz > 2) {
    fprintf(stderr, "phsz %d keybits:%d\n", phsz, head.keybits);
    fprintf(stderr, "keymap '%s'\n", head.keymap);
    fprintf(fp_out,TSIN_GTAB_KEY" %d %d %s\n", head.keybits, head.maxkey, head.keymap+1);
  }

  while (!feof(fp)) {
    phokey_t phbuf[MAX_PHRASE_LEN];
    u_int phbuf32[MAX_PHRASE_LEN];
    u_int64_t phbuf64[MAX_PHRASE_LEN];
    gboolean is_deleted = FALSE;

    fread(&clen,1,1,fp);
    if (clen < 0) {
      clen = - clen;
      is_deleted = TRUE;
    }

    fread(&usecount, sizeof(usecount_t), 1,fp);
    if (!pr_usecount)
      usecount = 0;

    if (phsz==2)
      fread(phbuf, sizeof(phokey_t), clen, fp);
    else
    if (phsz==4)
      fread(phbuf32, 4, clen, fp);
    else
    if (phsz==8)
      fread(phbuf64, 8, clen, fp);


    char tt[512];
    int ttlen=0;
    tt[0]=0;
    for(i=0;i<clen;i++) {
      char ch[CH_SZ];

      int n = fread(ch, 1, 1, fp);
      if (n<=0)
        goto stop;

      int len=utf8_sz(ch);

      fread(&ch[1], 1, len-1, fp);

      memcpy(tt+ttlen, ch, len);
      ttlen+=len;
    }
    tt[ttlen]=0;

    if (!tt[0])
      continue;

    if (is_deleted)
      continue;

    fprintf(fp_out, "%s ", tt);

    for(i=0;i<clen;i++) {
      if (phsz==2) {
        if (b_pinyin) {
          char *t = phokey2pinyin(phbuf[i]);
//          dbg("z %s\n", t);
          fprintf(fp_out, "%s", t);
        } else
          prph2(fp_out, phbuf[i]);
      } else {
        u_int64_t k;
        if (phsz==4)
          k = phbuf32[i];
        else
          k = phbuf64[i];

        char tkey[16];
        get_keymap_str(k, head.keymap, head.keybits, tkey);
        fprintf(fp_out, "%s", tkey);
      }

      if (i!=clen-1)
        fprintf(fp_out, " ");
    }

    fprintf(fp_out, " %d\n", usecount);
  }

stop:
  fclose(fp);
  fclose(fp_out);

  exit(0);
}
