/* Copyright (C) 2010 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "t2s-file.h"
#include "util.h"

T2S t2s[3000],s2t[3000];
int t2sn;

int qcmp(const void *aa0, const void *bb0)
{
  T2S *aa = (T2S *)aa0;
  T2S *bb = (T2S *)bb0;
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
  /* This data file is maintained by caleb-, ONLY for conversion
   * from Traditional Chinese to Simplified Chinese.
   * (Single Chinese glyph, one to one conversion.)
   *
   * However, "hime-sim2trad" also use this file to do "S to T"
   * conversion, so the conversion result is not very ideal.
   */
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
