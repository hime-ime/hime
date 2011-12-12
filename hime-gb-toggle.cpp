#include "hime.h"

int main(int argc, char **argv)
{
  gdk_init(NULL, NULL);

  /* Force to output original string, usually are Traditional Chinese */
  if (strstr(argv[0],"hime-trad"))
    send_hime_message(GDK_DISPLAY(), TRAD_OUTPUT_TOGGLE);

  /* Force to output Simplified Chinese */
  if (strstr(argv[0],"hime-sim"))
    send_hime_message(GDK_DISPLAY(), SIM_OUTPUT_TOGGLE);

  /* Toggle between Original string and Simplified Chinese */
  if (strstr(argv[0],"hime-gb-toggle"))
    send_hime_message(GDK_DISPLAY(), GB_OUTPUT_TOGGLE);

  return 0;
}
