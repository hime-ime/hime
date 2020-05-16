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
 @file hime-gb-toggle.c
 @brief Send control messages to hime server

 Provides:
   hime-gb-toggle
   hime-trad
   hime-sim
   hime-exit
   hime-kbm-toggle

*/

#include "hime.h"

#include "hime-im-client.h"

int main (int argc, char **argv) {
    gdk_init (NULL, NULL);

    /* Force to output original string, usually are Traditional Chinese */
    if (strstr (argv[0], "hime-trad"))
        send_hime_message (GDK_DISPLAY (), TRAD_OUTPUT_TOGGLE);

    /* Force to output Simplified Chinese */
    if (strstr (argv[0], "hime-sim"))
        send_hime_message (GDK_DISPLAY (), SIM_OUTPUT_TOGGLE);

    /* Toggle between Original string and Simplified Chinese */
    if (strstr (argv[0], "hime-gb-toggle"))
        send_hime_message (GDK_DISPLAY (), GB_OUTPUT_TOGGLE);

    /* Toggle virtual keyboard */
    if (strstr (argv[0], "hime-kbm-toggle"))
        send_hime_message (GDK_DISPLAY (), KBM_TOGGLE);

    if (strstr (argv[0], "hime-exit")) {
        Display *dpy = GDK_DISPLAY ();
        if (find_hime_window (dpy) == None)
            return 0;
        send_hime_message (dpy, HIME_EXIT_MESSAGE);
    }

    return 0;
}
