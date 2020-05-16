/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
 @file hime-exec-script.c
 @brief Create default settings in $HOME

 mkdir -p $HOME/.config/hime/config
 cp {essential files} to $HOME/.config/hime

*/

#include <stdio.h>
#include <stdlib.h>

#include <pwd.h>

#include "hime.h"

static void exec_script (char *name) {
    char scr[512];
    snprintf (scr, sizeof (scr), HIME_SCRIPT_DIR "/%s", name);
    dbg ("do %s\n", scr);
    system (scr);
}

void exec_setup_scripts () {
    /* Workaround to prevent hime-setup segfault, when hime/config/ is not exist.
   */
    struct passwd *pw = getpwuid (getuid ());
    char hime_conf_dir[512];
    g_snprintf (hime_conf_dir, sizeof (hime_conf_dir), "mkdir -p %s/.config/hime/config", pw->pw_dir);
    dbg ("do %s\n", hime_conf_dir);
    system (hime_conf_dir);
    exec_script ("hime-user-setup " HIME_TABLE_DIR " " HIME_BIN_DIR);
}
