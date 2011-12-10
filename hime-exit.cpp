#include "hime.h"
#include "hime-im-client.h"

#if WIN32
 #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

int main()
{
  gdk_init(NULL, NULL);

#if UNIX
  Display *dpy = GDK_DISPLAY();
  if (find_hime_window(dpy)==None)
    return;
  send_hime_message(dpy, HIME_EXIT_MESSAGE);
#else
  if (!find_hime_window())
    return 0;
  send_hime_message(HIME_EXIT_MESSAGE);
#endif

  return 0;
}
