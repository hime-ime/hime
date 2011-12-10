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
