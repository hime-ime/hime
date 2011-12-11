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

#include <stdio.h>
#include "hime.h"
#include "pho.h"

int main()
{
  FILE *fp;
  char fnamein[]="pin-juyin.src";
  PIN_JUYIN pinju[1024];
  short pinjuN=0;

  if ((fp=fopen(fnamein, "r"))==NULL)
    p_err("cannot open %s", fnamein);

  while (!feof(fp)) {
    char tt[128];

    tt[0]=0;
    fgets(tt, sizeof(tt), fp);
    if (strlen(tt) < 3)
      break;

    char pin[16], ju[64];
    bzero(pin, sizeof(pin));
    sscanf(tt, "%s %s",pin, ju);

    phokey_t kk=0;
    int len = strlen(ju);
    int i=0;
    while (i<len) {
      kk |= lookup((u_char *)&ju[i]);
      i+=utf8_sz(&ju[i]);
    }

//    dbg("%s '%s' %d\n", pin, ju, kk);

    memcpy(pinju[pinjuN].pinyin, pin, sizeof(pinju[0].pinyin));
    pinju[pinjuN].key = kk;
    pinjuN++;
  }

  fclose(fp);
  dbg("zz pinjuN:%d\n", pinjuN);

  char fnameout[]="pin-juyin.xlt";

  if ((fp=fopen(fnameout, "wb"))==NULL)
    p_err("cannot create %s", fnameout);

  fwrite(&pinjuN, sizeof(pinjuN), 1, fp);
  fwrite(pinju, sizeof(PIN_JUYIN), pinjuN, fp);
  fclose(fp);

  return 0;
}
