/* Copyright (C) 2009 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
#include "t2s-file.h"
#include "hime.h"

static int N;
static FILE *fp;

static int k_lookup(char *s, char out[])
{
  unsigned int key;
  bzero(&key, sizeof(key));

  u8cpy((char *)&key, s);

  int bot=0, top=N-1;
  while (bot <= top) {
    int mid = (bot + top)/2;
    T2S t;
    fseek(fp, mid*sizeof(T2S), SEEK_SET);
    fread(&t, sizeof(T2S), 1, fp);

    if (key > t.a)
      bot=mid+1;
    else
    if (key < t.a)
      top=mid-1;
    else
      return u8cpy(out, (char *)&t.b);
  }

  return u8cpy(out, s);
}

#include <sys/stat.h>

static int translate(char *fname, char *str, int strN, char **out)
{
  char fullname[128];

  get_sys_table_file_name(fname, fullname);

  if ((fp=fopen(fullname, "rb"))==NULL)
    p_err("cannot open %s %s", fname, fullname);

  struct stat st;

  stat(fullname, &st);
  N = st.st_size / sizeof(T2S);

  char *p=str;
  char *endp = str + strN;
  int opN=0;
  char *op = NULL;

  while (p < endp) {
    op = (char *)realloc(op, opN+5);
    opN += k_lookup(p, &op[opN]);
    p+=utf8_sz(p);
  }

  fclose(fp);
  *out = op;
  op[opN]=0;
  return opN;
}

int trad2sim(char *str, int strN, char **out)
{
  return translate("t2s.dat", str, strN, out);
}


int sim2trad(char *str, int strN, char **out)
{
  puts(str);
  return translate("s2t.dat", str, strN, out);
  puts(*out);
}
