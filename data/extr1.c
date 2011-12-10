#include <stdio.h>
#include <string.h>
#include "../util.h"
int utf8_str_N(char *str);


int main()
{
  FILE *fp;

  if ((fp=fopen("tsin.src", "r"))==NULL)
    p_err("cannot open");

  while (!feof(fp)) {
    char aa[128];
    char bb[128];
    int usecount;
    char line[256];

    fgets(line, sizeof(line), fp);
    sscanf(line, "%s %s %d", aa, bb, &usecount);

    if (utf8_str_N(aa)==1)
      printf("%s", line);
  }

  return 0;
}
