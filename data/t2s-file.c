#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../t2s-file.h"
#include "../util.h"

T2S t2s[3000],s2t[3000];
int t2sn;

int qcmp(T2S *aa, T2S *bb)
{
#if 0
  int64_t a = aa->a;
  int64_t b = bb->a;
#else
  u_int a = aa->a;
  u_int b = bb->a;
#endif

  if (a > b)
    return 1;
  if (a < b)
    return -1;
  return 0;
}

void gen(T2S *t, char *name)
{
  qsort(t, t2sn, sizeof(T2S), qcmp);
  FILE *fw;

  if ((fw=fopen(name,"w"))==NULL)
    p_err("cannot write %s", name);
  fwrite(t, sizeof(T2S), t2sn, fw);
  fclose(fw);
}

int main()
{
  char *fname="t2s-file.table";
  FILE *fp=fopen(fname, "r");

  if (!fp)
    dbg("cannot open %s", fname);

  while (!feof(fp)) {
    char tt[128];
    tt[0]=0;
    fgets(tt, sizeof(tt), fp);
    if (!tt[0])
      break;
    char a[9],b[9];

    bzero(a, sizeof(a));
    bzero(b, sizeof(b));
    sscanf(tt,"%s %s",a,b);
    memcpy(&t2s[t2sn].a, a, sizeof(t2s[0].a));
    memcpy(&t2s[t2sn].b, b, sizeof(t2s[0].b));
    memcpy(&s2t[t2sn].b, a, sizeof(s2t[0].a));
    memcpy(&s2t[t2sn].a, b, sizeof(s2t[0].b));
    t2sn++;
//    dbg("%s %s\n", a,b);
  }

  gen(t2s, "t2s.dat");
  gen(s2t, "s2t.dat");

  return 0;
}
