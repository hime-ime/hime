#include "hime.h"
#include "gtab.h"
#include "hime-conf.h"
#include "hime-endian.h"
#include "pho.h"
#include "tsin.h"
#include "tsin-parse.h"
#if WIN32
#include <io.h>
#endif

void get_hime_user_or_sys_fname(char *name, char fname[]);

gboolean init_tsin_table_fname(INMD *p, char *fname)
{
  char fname_idx[256], gtab_phrase_src[256], gtabfname[256];
  if (p->filename_append) {
//    dbg("filename_append %s\n",p->filename_append);
    strcpy(fname, p->filename_append);
    strcpy(gtabfname, fname);
  } else
  if (p->filename) {
    get_hime_user_fname(p->filename, fname);
    get_hime_user_or_sys_fname(p->filename, gtabfname);
  } else {
    dbg("no file name\n");
    return FALSE;
  }

  strcat(fname, ".tsin-db");
  strcat(strcpy(fname_idx, fname), ".idx");
  strcat(strcpy(gtab_phrase_src, fname), ".src");
//  dbg("init_tsin_table %s\n", fname);

#if UNIX
  putenv("HIME_NO_RELOAD=");
#else
  _putenv("HIME_NO_RELOAD=Y");
#endif

#if UNIX
  if (access(fname, W_OK) < 0 || access(fname_idx, W_OK) < 0)
#else
  if (_access(fname, 02) < 0 || _access(fname, 02) < 0)
#endif
  {
#if UNIX
    unix_exec(HIME_BIN_DIR"/hime-tsin2gtab-phrase %s %s", gtabfname, gtab_phrase_src);
    unix_exec(HIME_BIN_DIR"/hime-tsa2d32 %s %s", gtab_phrase_src, fname);
#else
    win32exec_va("hime-tsin2gtab-phrase", gtabfname, gtab_phrase_src, NULL);
    win32exec_va("hime-tsa2d32", gtab_phrase_src, fname, NULL);
#endif
  }

  return TRUE;
}
