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
#include "gtab.h"

/* this function is used to avoid 4-byte bus-alignment */
u_int64_t CONVT2(INMD *inmd, int i)
{
  u_int64_t kk;

  if (i >= inmd->DefChars || i < 0) {
//    dbg("%d %d\n", i, inmd->DefChars);
    return 0;
  }

  if (inmd->key64) {
    memcpy(&kk, inmd->tbl64[i].key, sizeof(u_int64_t));
  }
  else {
    u_int tt;
    memcpy(&tt, inmd->tbl[i].key, sizeof(u_int));
    kk = tt;
  }

  return kk;
}

int gtab_key2name(INMD *tinmd, u_int64_t key, char *t, int *rtlen)
{
    int tlen=0, klen=0;

    int j;
    for(j=Max_tab_key_num1(tinmd) - 1; j>=0; j--) {
      int sh = j * KeyBits1(tinmd);
      int k = (key >> sh) & tinmd->kmask;

      if (!k)
        break;
      int len;
      char *keyname;

      if (tinmd->keyname_lookup) {
        len = 1;
        keyname = (char *)&tinmd->keyname_lookup[k];
      } else {
        keyname = (char *)&tinmd->keyname[k * CH_SZ];
        len = (*keyname & 0x80) ? utf8_sz(keyname) : strlen(keyname);
      }
//      dbg("uuuuuuuuuuuu %d %x len:%d\n", k, tinmd->keyname[k], len);
      memcpy(&t[tlen], keyname, len);
      tlen+=len;
      klen++;
    }

    t[tlen]=0;
    *rtlen = tlen;
    return klen;
}
