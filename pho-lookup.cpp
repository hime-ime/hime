#include "hime.h"
#include "pho.h"

static int shiftb[]={9,7,3,0};

int lookup(u_char *s)
{
  int i;
  char tt[CH_SZ+1], *pp;

  if (*s < 128)
    return *s-'0';

  bchcpy(tt, s);
  tt[PHO_CHAR_LEN]=0;

  for(i=0;i<3;i++)
    if ((pp=strstr(pho_chars[i],tt)))
      break;

  if (i==3)
    return 0;

  return (((pp-pho_chars[i])/3) << shiftb[i]);
}
