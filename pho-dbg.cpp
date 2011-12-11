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

void utf8_putchar_fp(FILE *fp, char *s);
void prph2(FILE *fp, phokey_t kk)
{
  u_int k[4];
  phokey_t okk = kk;

  k[3]=(kk&7);
  kk>>=3;
  k[2]=(kk&15) * PHO_CHAR_LEN;
  kk>>=4;
  k[1]=(kk&3) * PHO_CHAR_LEN;
  kk>>=2;
  k[0]=(kk&31) * PHO_CHAR_LEN;


  if (k[0]==BACK_QUOTE_NO*PHO_CHAR_LEN) {
    utf8_putchar(&pho_chars[0][k[0]]);
    char c = okk & 0x7f;
    if (c > ' ')
      fprintf(fp, "%c", c);
  } else {
    int i;
    for(i=0; i < 3; i++) {
      if (!k[i])
        continue;

      utf8_putchar_fp(fp, &pho_chars[i][k[i]]);
    }

    if (k[3])
      fprintf(fp, "%d", k[3]);
  }
}


void prph(phokey_t kk)
{
	prph2(stdout, kk);
}


void prphs(phokey_t *ks, int ksN)
{
  int i;
  for(i=0;i<ksN;i++) {
    prph(ks[i]); dbg(" ");
  }
}

