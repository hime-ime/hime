#include "hime-protocol.h"

static int __hime_rand__(u_int *next)
{
  *next = *next * 1103515245 + 12345;
  return((unsigned)(*next/65536) % 32768);
}

void __hime_enc_mem(u_char *p, int n,
                    HIME_PASSWD *passwd, u_int *seed)
{
  int i;

  for(i=0; i < n; i++) {
    int v = __hime_rand__(seed) % __HIME_PASSWD_N_;
    p[i]^=passwd->passwd[v];
  }
}

