/* Copyright (C) 2009 Edward Der-Hua Liu, Hsin-Chu, Taiwan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation version 2.1
 * of the License.
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
#include "hime-conf.h"
void sys_icon_fname(char *iconame, char fname[])
{
  if(!strcmp(hime_icon_dir, "DEFAULT")) {
    sprintf(fname, HIME_DEFAULT_ICON_DIR"/%s", iconame);
  }
  else {
    sprintf(fname, HIME_DEFAULT_ICON_DIR"/%s/%s", hime_icon_dir, iconame);
  }
}

void get_icon_path(char *iconame, char fname[])
{
  char uu[128];
  sprintf(uu, "icons/%s", iconame);

  if (!get_hime_user_fname(uu, fname)) {
    sys_icon_fname(iconame, fname);
  }
}


void set_window_hime_icon(GtkWidget *window)
{
  gtk_window_set_icon_from_file(GTK_WINDOW(window), SYS_ICON_DIR"/hime.png", NULL);
}
