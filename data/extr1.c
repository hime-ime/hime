/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include <stdio.h>
#include <string.h>

#include "hime.h"

int main (void) {
    FILE *fp = fopen ("tsin.src", "r");

    if (!fp)
        p_err ("cannot open");

    while (!feof (fp)) {
        char aa[128];
        char bb[128];
        int usecount;
        char line[256];

        fgets (line, sizeof (line), fp);
        sscanf (line, "%s %s %d", aa, bb, &usecount);

        if (utf8_str_N (aa) == 1)
            printf ("%s", line);
    }

    return 0;
}
