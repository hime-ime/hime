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

#include "hime.h"
#include "gtab.h"

typedef struct {
  int use_count;
  u_char bytes, flag;
} GTAB_USE_CNT;

static char gtab_use_count_file[]="gtab-use-count2";
static FILE *fp_gtab_use_count;

static void init_fp()
{
  if (!fp_gtab_use_count) {
    char fname[128];
    get_hime_user_fname(gtab_use_count_file, fname);

    if (!(fp_gtab_use_count=fopen(fname, "rb+"))) {
      if (!(fp_gtab_use_count=fopen(fname, "wb+"))) {
        dbg("cannot write to %s\n", fname);
        return;
      }
    }
  }
}


void inc_gtab_use_count(char *s)
{
  init_fp();

  int bytes = strlen(s);

  GTAB_USE_CNT c;
  rewind(fp_gtab_use_count);

//  dbg("zzz '%s' bytes:%d\n", s, bytes);
//  dbg("inc %d\n", ftell(fp_gtab_use_count));
  while (!feof(fp_gtab_use_count)) {
    char tt[512];
    bzero(&c, sizeof(c));
    fread(&c, sizeof(c), 1, fp_gtab_use_count);
    if (c.bytes != bytes)
      continue;
    fread(tt, bytes, 1, fp_gtab_use_count);

    if (memcmp(tt, s, bytes))
      continue;

//    long ofs = ftell(fp_gtab_use_count);
//    dbg("aa %d ofs:%d sz:%d\n", c.use_count, ofs, sizeof(c));
    fseek(fp_gtab_use_count, - (sizeof(c)+bytes) , SEEK_CUR);
//    dbg("bb %d ofs:%d\n", c.use_count, ftell(fp_gtab_use_count));

    c.use_count++;
    fwrite(&c, sizeof(c), 1, fp_gtab_use_count);
    fflush(fp_gtab_use_count);
    return;
  }

//  int fofs = ftell(fp_gtab_use_count);
//  dbg("fofs: %d\n", fofs);

#if 0
  int delta = fofs % sizeof(GTAB_USE_CNT);
  if (delta) // avoid incomplete write
    fseek(fp_gtab_use_count, - delta, SEEK_CUR);
#endif

  bzero(&c, sizeof(c));
  c.use_count = 1;
  c.bytes = bytes;
  fwrite(&c, sizeof(c), 1, fp_gtab_use_count);
  fwrite(s, bytes, 1, fp_gtab_use_count);
  fflush(fp_gtab_use_count);
}


int get_gtab_use_count(char *s)
{
  int bytes = strlen(s);
  init_fp();

//  dbg("get_gtab_use_count %s\n", s);

  GTAB_USE_CNT c;
  rewind(fp_gtab_use_count);
  while (!feof(fp_gtab_use_count)) {
    fread(&c, sizeof(c), 1, fp_gtab_use_count);
    if (c.bytes != bytes)
      continue;
    char tt[512];
    fread(tt, bytes, 1, fp_gtab_use_count);

    if (!memcmp(tt, s, bytes)) {
//      dbg("count found %s %d\n", s, c.use_count);
      return c.use_count;
    }
  }

//  dbg("not found\n");

  return 0;
}
