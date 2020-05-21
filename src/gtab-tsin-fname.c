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

#include "gtab.h"
#include "hime-conf.h"
#include "hime-endian.h"
#include "pho.h"
#include "tsin.h"

gboolean init_tsin_table_fname (INMD *p, char *fname) {
    char fname_idx[256], gtab_phrase_src[256], gtabfname[256];
    if (p->filename_append) {
        //    dbg("filename_append %s\n",p->filename_append);
        strcpy (fname, p->filename_append);
        strcpy (gtabfname, fname);
    } else if (p->filename) {
        get_hime_user_fname (p->filename, fname);
        get_hime_user_or_sys_fname (p->filename, gtabfname);
    } else {
        dbg ("no file name\n");
        return FALSE;
    }

    strcat (fname, ".tsin-db");
    strcat (strcpy (fname_idx, fname), ".idx");
    strcat (strcpy (gtab_phrase_src, fname), ".src");
    //  dbg("init_tsin_table %s\n", fname);

    setenv ("HIME_NO_RELOAD", "", TRUE);

    if (access (fname, W_OK) < 0 || access (fname_idx, W_OK) < 0) {
        unix_exec (HIME_BIN_DIR "/hime-tsin2gtab-phrase %s %s", gtabfname, gtab_phrase_src);
        unix_exec (HIME_BIN_DIR "/hime-tsa2d32 %s %s", gtab_phrase_src, fname);
    }

    return TRUE;
}
