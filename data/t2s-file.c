/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 2010 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
#include <stdlib.h>
#include <string.h>

#include "t2s-file.h"
#include "util.h"

#define SIZE 3000

static T2S t2s[SIZE], s2t[SIZE];
static int t2sn;

static int qcmp (const void *aa0, const void *bb0) {
    const T2S *aa = (const T2S *) aa0;
    const T2S *bb = (const T2S *) bb0;

    if (aa->a > bb->a)
        return 1;
    if (aa->a < bb->a)
        return -1;
    return 0;
}

static void gen (T2S *t, const char *name) {
    qsort (t, t2sn, sizeof (T2S), qcmp);
    FILE *fw = fopen (name, "w");

    if (!fw)
        p_err ("cannot write %s", name);

    fwrite (t, sizeof (T2S), t2sn, fw);
    fclose (fw);
}

int main (void) {
    /*
     * This data file is maintained by caleb-, ONLY for conversion
     * from Traditional Chinese to Simplified Chinese.
     * (Single Chinese glyph, one to one conversion.)
     *
     * However, "hime-sim2trad" also use this file to do "S to T"
     * conversion, so the conversion result is not very ideal.
     */
    t2sn = 0;
    const char *fname = "t2s-file.table";
    FILE *fp = fopen (fname, "r");

    if (!fp)
        dbg ("cannot open %s", fname);

    while (!feof (fp) && t2sn < SIZE) {
        char tt[128];
        tt[0] = '\0';

        fgets (tt, sizeof (tt), fp);
        if (!tt[0])
            break;

        char a[9], b[9];
        memset (a, 0, sizeof (a));
        memset (b, 0, sizeof (b));

        sscanf (tt, "%s %s", a, b);

        memcpy (&t2s[t2sn].a, a, sizeof (t2s[0].a));
        memcpy (&t2s[t2sn].b, b, sizeof (t2s[0].b));
        memcpy (&s2t[t2sn].b, a, sizeof (s2t[0].a));
        memcpy (&s2t[t2sn].a, b, sizeof (s2t[0].b));

        t2sn++;
    }

    gen (t2s, "t2s.dat");
    gen (s2t, "s2t.dat");

    return 0;
}
