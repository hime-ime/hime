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

void get_keymap_str(u_int64_t k, char *keymap, int keybits, char tkey[])
{
  int tkeyN=0;
  u_int mask = ((1 << keybits) - 1);

  while (k) {
    int v = k & mask;
    if (v)
      tkey[tkeyN++] = keymap[v];
    k>>=keybits;
  }
  tkey[tkeyN]=0;

  int j;
  for(j=0;j<tkeyN/2;j++) {
    char t = tkey[j];
    tkey[j]=tkey[tkeyN-j-1];
    tkey[tkeyN-j-1] = t;
  }
}
