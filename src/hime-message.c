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

#include "hime.h"

int main (int argc, char **argv) {
    int i;
    char text[128];
    char icon[128];
    int duration = 3000;

    gdk_init (&argc, &argv);

    if (argc < 3)
        p_err ("usage: hime-message -icon file_name -text string -duration milli_seconds\n");

    strcpy (text, "-");
    strcpy (icon, "-");

    for (i = 1; i < argc; i += 2) {
        if (!strcmp (argv[i], "-icon")) {
            strcpy (icon, argv[i + 1]);
        } else if (!strcmp (argv[i], "-text")) {
            strcpy (text, argv[i + 1]);
        } else if (!strcmp (argv[i], "-duration")) {
            duration = atoi (argv[i + 1]);
        } else {
            dbg ("unknown opt %s", argv[i]);
        }
    }

    char message[512];

    snprintf (message, sizeof (message), "#hime_message %s %s %d", icon, text, duration);

    send_hime_message (GDK_DISPLAY (), message);

    return 0;
}
