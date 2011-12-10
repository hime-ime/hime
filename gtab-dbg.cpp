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
