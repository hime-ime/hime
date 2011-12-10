#include "hime.h"
#if WIN32
#include <io.h>
#endif

void sys_icon_fname(char *iconame, char fname[])
{
#if WIN32
  sprintf(fname, "%s\\icons\\%s",hime_program_files_path, iconame);
#else
  sprintf(fname, HIME_ICON_DIR"/%s", iconame);
#endif
}

void get_icon_path(char *iconame, char fname[])
{
  char uu[128];
#if UNIX
  sprintf(uu, "icons/%s", iconame);
#else
  sprintf(uu, "icons\\%s", iconame);
#endif
  get_hime_user_fname(uu, fname);

#if UNIX
  if (access(fname, R_OK)) {
#else
  if (_access(fname, 04)) {
#endif
    sys_icon_fname(iconame, fname);
  }
}


void set_window_hime_icon(GtkWidget *window)
{
#if WIN32
  char tt[128];
  sys_icon_fname("hime.png", tt);
  gtk_window_set_icon_from_file(GTK_WINDOW(window), tt, NULL);
#else
  gtk_window_set_icon_from_file(GTK_WINDOW(window), SYS_ICON_DIR"/hime.png", NULL);
#endif
}
