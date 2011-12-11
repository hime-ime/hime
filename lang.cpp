/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

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
