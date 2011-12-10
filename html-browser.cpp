#include "hime.h"
#include "gtab.h"
#include "config.h"
#if UNIX
#include <signal.h>
#endif

#if UNIX
char html_browse[]=HIME_SCRIPT_DIR"/html-browser";
#endif

int html_browser(char *fname)
{
#if WIN32
  LONG r = (LONG)ShellExecuteA(NULL, "open", fname, NULL, NULL, SW_SHOWNORMAL);
  return r;
#else
  char tt[256];
  sprintf(tt, "%s %s", html_browse, fname);
  dbg("%s\n", tt);
  return system(tt);
#endif
}

