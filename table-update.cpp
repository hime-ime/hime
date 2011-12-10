#include "hime.h"
#include <sys/stat.h>

void update_table_file(char *name, int version)
{
#if UNIX
  char fname_user[256];
  char fname_version[256];
  char fname_sys[256];
  char version_name[256];

  strcat(strcpy(version_name, name), ".version");
  get_hime_user_fname(version_name, fname_version);
  get_hime_user_fname(name, fname_user);
  get_sys_table_file_name(name, fname_sys);

  FILE *fp;
  if ((fp=fopen(fname_version, "r"))) {
    int ver=0;
    fscanf(fp, "%d", &ver);
    fclose(fp);

    if (ver >= version)
      return;
  }

  char cmd[256];
  sprintf(cmd, "mv -f %s %s.old && cp %s %s && echo %d > %s", fname_user, fname_user,
      fname_sys, fname_user, version, fname_version);
  dbg("exec %s\n", cmd);
  system(cmd);
#endif
}
