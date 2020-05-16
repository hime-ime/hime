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

#include "pho.h"

void load_tab_pho_file ();

#include <signal.h>
gboolean test_mode;

int pho_play (phokey_t key) {
    if (!phonetic_speak)
        return 0;
    if (test_mode)
        return 0;

    static int pid;
    static time_t last_time;
    time_t t = time (NULL);
    if (!hime_sound_play_overlap) {
        if (pid && t - last_time < 2)
            kill (pid, 9);
    }
    char *ph = phokey_to_str2 (key, 1);
    char tt[512];

    last_time = t;
    snprintf (tt, sizeof (tt), HIME_OGG_DIR "/%s/%s", ph, phonetic_speak_sel);

    if (access (tt, R_OK))
        return 0;

    if ((pid = fork ())) {
        if (pid < 0)
            dbg ("cannot fork ?");
        return 1;
    }

    close (1);
    close (2);
    execlp ("ogg123", "ogg123", tt, NULL);
    return 0;
}

void char_play (char *utf8) {
    if (!phonetic_speak || !(utf8[0] & 128))
        return;

    if (!ch_pho)
        load_tab_pho_file ();

    phokey_t phos[16];
    int phosN = utf8_pho_keys ((char *) utf8, phos);

    if (!phosN)
        return;

#if 0
  int i;
  for(i=0; i < phosN; i++)
    pho_play(phos[i]);
#else
    pho_play (phos[0]);
#endif
}
