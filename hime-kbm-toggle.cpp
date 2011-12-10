#include "hime.h"

int main()
{
  gdk_init(NULL, NULL);
  send_hime_message(GDK_DISPLAY(), KBM_TOGGLE);

  return 0;
}
