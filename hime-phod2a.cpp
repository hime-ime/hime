/*
	Copyright (C) 1994-2005	Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
