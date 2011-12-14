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

#include <stdio.h>
#include <stdlib.h>
#include "os-dep.h"
#include "hime.h"

#if UNIX
static void exec_script(char *name)
{
  char scr[512];

  sprintf(scr, HIME_SCRIPT_DIR"/%s", name);
  system(scr);
}
#endif

void exec_setup_scripts()
{
#if WIN32
win32exec_script("hime-user-setup.bat");

char hime_table[128];
strcat(strcpy(hime_table, getenv("HIME_DIR")), "\\table");
char *app_hime=getenv("APPDATA_HIME");

char *files[]={
"pho.tab2", "pho-huge.tab2",
"s-pho.tab2", "s-pho-huge.tab2",
"tsin32", "tsin32.idx",
"s-tsin32", "s-tsin32.idx",
"symbol-table", "phrase.table"};
for(int i=0; i < sizeof(files)/sizeof(files[0]); i++) {
	char src[MAX_PATH], dest[MAX_PATH];
	sprintf(src, "%s\\%s", hime_table, files[i]);
	sprintf(dest, "%s\\%s", app_hime, files[i]);

//	dbg("%s -> %s\n", src, dest);
	CopyFileA(src, dest, true);
}
#else
  exec_script("hime-user-setup "HIME_TABLE_DIR" "HIME_BIN_DIR);
#endif
}
