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

#include <errno.h>

#include "hime.h"

#if !CLIENT_LIB && DEBUG
static FILE *out_fp;
#endif

void p_err (char *fmt, ...) {
    va_list args;
    char out[4096];

    va_start (args, fmt);
    vsprintf (out, fmt, args);
    va_end (args);

    fprintf (stderr, "%s\n", out);

#if DEBUG
    abort ();
#else
    if (getenv ("HIME_ERR_COREDUMP"))
        abort ();
    exit (-1);
#endif
}

#if !CLIENT_LIB && DEBUG
static void init_out_fp (void) {
    if (!out_fp) {
        if (getenv ("HIME_DBG_TMP") || 0) {
            char fname[64];
            snprintf (fname, sizeof (fname), "%s/himedbg-%d-%d", g_get_tmp_dir (), getuid (), getpid ());
            out_fp = fopen (fname, "w");
        }

        if (!out_fp)
            out_fp = stdout;
    }
}
#endif

#if !CLIENT_LIB
void dbg_time (char *fmt, ...) {
#if DEBUG
    va_list args;
    time_t t;

    init_out_fp ();

    time (&t);
    struct tm *ltime = localtime (&t);
    dbg ("%02d:%02d:%02d ", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

    va_start (args, fmt);
    vfprintf (out_fp, fmt, args);
    fflush (out_fp);
    va_end (args);
#endif
}
#endif

#if DEBUG
void __hime_dbg_ (char *fmt, ...) {
    va_list args;

    init_out_fp ();

    va_start (args, fmt);
    vfprintf (out_fp, fmt, args);
    fflush (out_fp);
    va_end (args);
}
#endif

char *sys_err_strA (void) {
    return (char *) strerror (errno);
}

void *zmalloc (int n) {
    void *p = malloc (n);
    memset (p, 0, n);
    return p;
}
#if !HIME_IME

void *memdup (void *p, int n) {
    if (!p || !n)
        return NULL;
    void *q;
    q = malloc (n);
    memcpy (q, p, n);
    return q;
}

// can handle eol with \n \r \n\r \r\n
char *myfgets (char *buf, int bufN, FILE *fp) {
    char *out = buf;
    //	int rN = 0;
    while (!feof (fp) && out - buf < bufN) {
        char a, b;
        a = 0;
        if (fread (&a, 1, 1, fp) != 1)
            break;
        if (a == '\n') {
            b = 0;
            if (fread (&b, 1, 1, fp) == 1)
                if (b != '\r')
                    fseek (fp, -1, SEEK_CUR);
            break;
        } else if (a == '\r') {
            b = 0;
            if (fread (&b, 1, 1, fp) == 1)
                if (b != '\n')
                    fseek (fp, -1, SEEK_CUR);
            break;
        }

        *(out++) = a;
    }

    *out = 0;
    return buf;
}
#endif
