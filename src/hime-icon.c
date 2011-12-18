/* Copyright (C) 2009 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
