#include "hime.h"
#include "hime-im-client.h"

#if UNIX
void send_hime_message(Display *dpy, char *s)
#else
void send_hime_message(char *s)
#endif
{
#if UNIX
  HIME_client_handle *handle = hime_im_client_open(dpy);
#else
  HIME_client_handle *handle = hime_im_client_open(NULL);
#endif
  hime_im_client_message(handle, s);

  hime_im_client_close(handle);
}
