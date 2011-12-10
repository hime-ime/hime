#include "hime.h"
#include "lang.h"

gboolean is_chs;
char *tsin32_f="tsin32";

void set_is_chs()
{
#if UNIX
  char *lc_ctype = getenv("LC_CTYPE");
  char *lc_all = getenv("LC_ALL");
  char *lang = getenv("LANG");
  if (!lc_ctype && lang)
    lc_ctype = lang;

  if (lc_all)
    lc_ctype = lc_all;

  if (!lc_ctype)
    lc_ctype = "zh_TW.Big5";
  dbg("hime get env LC_CTYPE=%s  LC_ALL=%s  LANG=%s\n", lc_ctype, lc_all, lang);

  if (strstr(lc_ctype, "zh_CN") || 0) {
    is_chs = TRUE;
  }
#else
  is_chs = GetACP() == 936;
//  is_chs = TRUE;
#endif

  if (is_chs) {
    tsin32_f = "s-tsin32";
    dbg("is simplified chinese\n");
  }
}
