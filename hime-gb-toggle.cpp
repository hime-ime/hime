#include "hime.h"

int main()
{
  gdk_init(NULL, NULL);
  send_hime_message(GDK_DISPLAY(), GB_OUTPUT_TOGGLE);

  return 0;
}
