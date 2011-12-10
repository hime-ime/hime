#include "hime.h"

void unix_exec(char *fmt,...)
{
  va_list args;
  char tt[512];

  va_start(args, fmt);
  vsnprintf(tt,sizeof(tt), fmt, args);
  va_end(args);
  printf("exec %s\n", tt);
  system(tt);
}
