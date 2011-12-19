/*
 * Copyright (C) 1994-2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
#include "hime-conf.h"

gboolean is_chs;

int main(int argc, char **argv)
{
  int i;

  gtk_init(&argc, &argv);

  load_setttings();

  if (argc > 1) {
    p_err("Currently only support ~/.config/hime/pho.tab2");
  }

  pho_load();

  for(i=0; i < idxnum_pho; i++) {
    phokey_t key = idx_pho[i].key;
    int frm = idx_pho[i].start;
    int to = idx_pho[i+1].start;

    int j;
    for(j=frm; j < to; j++) {
      prph(key);
      char *str = pho_idx_str(j);
      dbg(" %s %d\n", str, ch_pho[j].count);
    }
  }

  return 0;
}
