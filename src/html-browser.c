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

