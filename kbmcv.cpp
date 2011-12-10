/*
	Copyright (C) 1995-2008	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include "hime.h"
#include "pho.h"

int pho_lookup(char *s, char *num, char *typ)
{
  int i;
  char tt[CH_SZ+1], *pp;
  int len = utf8_sz(s);

  if (utf8_eq(s, "１")) {
    *num = 0;
    *typ = 3;
    return TRUE;
  }

  if (!(*s&0x80))
    return *s-'0';

  bchcpy(tt, s);
  tt[len]=0;

  for(i=0;i<4;i++) {
    if ((pp=strstr(pho_chars[i], tt)))
      break;
  }

  if (!pp)
    return FALSE;

  *typ=i;
  *num=(pp - pho_chars[i])/3;

  return TRUE;
}

void swap_char(char *a, char *b)
{
  char t;

  t = *a;
  *a = *b;
  *b = t;
}

int main(int argc, char **argv)
{
  FILE *fp;
  char s[128];
  int i,len;
  PHOKBM phkb;
  char num, typ, chk;
  char fnamesrc[40];
  char fnameout[40];

  if (!getenv("NO_GTK_INIT"))
    gtk_init(&argc, &argv);

  if (argc < 2) {
    puts("file name expected");
    exit(1);
  }

  bzero(&phkb,sizeof(phkb));
  strcpy(fnameout,argv[1]);

  char *p;
  if ((p=strchr(fnameout, '.')))
    *p = 0;

  strcpy(fnamesrc,fnameout);
  strcat(fnamesrc,".kbmsrc");
  strcat(fnameout,".kbm");

  if ((fp=fopen(fnamesrc,"r"))==NULL) {
    printf("Cannot open %s\n", fnamesrc);
    exit(1);
  }

//  fgets(s,sizeof(s),fp);
//  len=strlen(s);
//  s[len-1]=0;
//  strcpy(phkb.selkey, s);
//  phkb.selkeyN = strlen(s);

  while (!feof(fp)) {
    s[0]=0;
    fgets(s,sizeof(s),fp);
    len=strlen(s);
    if (!len)
      break;

    if (s[len-1]=='\n')
      s[--len]=0;

    if (!len)
      break;

    if (!pho_lookup(s, &num, &typ))
      p_err("err found %s", s);

    int utf8sz = utf8_sz(s);
    chk=s[utf8sz + 1];

    if (chk>='A' && chk<='Z')
      chk+=32;

    for(i=0;i<3;i++) {
      if (!phkb.phokbm[(int)chk][i].num) {
        phkb.phokbm[(int)chk][i].num=num;
        phkb.phokbm[(int)chk][i].typ=typ;

//       printf("%c %d %d  i:%d\n", chk, num, typ, i);
        break;
      }
    }
  }
  fclose(fp);

  if (strstr(fnamesrc, "pinyin"))
    phkb.phokbm[' '][0].num=0;
    phkb.phokbm[' '][0].typ=3;

  if ((fp=fopen(fnameout,"w"))==NULL) {
    printf("Cannot create %s\n", fnameout);
    exit(1);
  }

  fwrite(&phkb,sizeof(phkb),1,fp);
  fclose(fp);
  exit(0);
}
