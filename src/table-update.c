/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 2009 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

/**
 @file table-update.c
 @brief Update pho.tab2 when necessary

 if (phonetic_char_dynamic_sequence) {
   mv pho.tab2 pho.tab2.old
   $(UPDATE) pho.tab2
   $(CREATE) pho.tab2.version
 }

*/

#include <sys/stat.h>

#include "hime.h"

void update_table_file (char *name, int version) {
    if (!phonetic_char_dynamic_sequence)
        return;

    char fname_user[256];
    char fname_version[256];
    char fname_sys[256];
    char version_name[256];

    strcat (strcpy (version_name, name), ".version");
    get_hime_user_fname (version_name, fname_version);
    get_hime_user_fname (name, fname_user);
    get_sys_table_file_name (name, fname_sys);

    FILE *fp;
    if ((fp = fopen (fname_version, "r"))) {
        int ver = 0;
        fscanf (fp, "%d", &ver);
        fclose (fp);

        if (ver >= version)
            return;
    }

    char cmd[2048];
    snprintf (cmd, sizeof (cmd), "mv -f %s %s.old && cp %s %s && echo %d > %s",
              fname_user, fname_user,
              fname_sys, fname_user, version, fname_version);
    dbg ("exec %s\n", cmd);
    system (cmd);
}
