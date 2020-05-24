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

#include <string.h>

#include "hime.h"

#include "im-srv.h"

Atom get_hime_addr_atom (Display *display) {
    return get_atom_by_name (display, "HIME_ADDR_ATOM_%s");
}

Atom get_hime_sockpath_atom (Display *display) {
    return get_atom_by_name (display, "HIME_SOCKPATH_ATOM_%s");
}

// socket name: /tmp/.hime-$USER/socket-:0.0-hime
void get_hime_im_srv_sock_path (char *outstr, const int outstrN) {
    const char *display = getenv ("DISPLAY");
    const int uid = getuid ();

    if (!display || (strcmp (display, ":0") == 0)) {
        display = ":0.0";
    }

    const int DISPLAY_NAME_SIZE = 64;
    char tdisplay[DISPLAY_NAME_SIZE];
    strncpy (tdisplay, display, sizeof (tdisplay));

    if (!strchr (display, ':')) {
        strcat (tdisplay, ":0");
    }
    if (!strchr (display, '.')) {
        strcat (tdisplay, ".0");
    }

    const int DIR_NAME_SIZE = 128;
    char my_dir[DIR_NAME_SIZE];

    struct passwd *pw = getpwuid (uid);
    const gchar *tmpdir = g_get_tmp_dir ();
    snprintf (my_dir, sizeof (my_dir), "%s/.hime-%s", tmpdir, pw->pw_name);
    struct stat st;

    // my_dir doesn't exist, create one
    if (stat (my_dir, &st) == -1) {
        mkdir (my_dir, 0700);
    } else {
        if (st.st_uid != uid) {
            fprintf (stderr, "please check the permission of dir %s\n", my_dir);
            return;
        }
    }

    snprintf (outstr, outstrN,
              "%s/socket-%s-%s",
              my_dir, tdisplay, get_hime_xim_name ());
}
