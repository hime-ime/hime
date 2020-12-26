/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 1995-2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#if FREEBSD
#include <sys/param.h>
#include <sys/stat.h>
#endif

#include "hime.h"

#include "gtab.h"
#include "hime-endian.h"

FILE *fr, *fw;
int lineno;
char tt[1024];

static char *skip_space (char *s) {
    while ((*s == ' ' || *s == '\t') && *s) {
        s++;
    }
    return s;
}

static char *to_space (char *s) {
    while (*s != ' ' && *s != '\t' && *s) {
        s++;
    }
    return s;
}

static void del_newline_space (char *s) {
    if (!*s) {
        return;
    }

    size_t len = strlen (s);
    char *t = s + len - 1;

    while (*t == '\n' || *t == ' ' || (*t == '\t' && t > s)) {
        t--;
    }

    *(t + 1) = 0;
}

static void get_line (void) {
    while (!feof (fr)) {
        memset (tt, 0, sizeof (tt));
        myfgets (tt, sizeof (tt), fr);

        lineno++;
        size_t len = strlen (tt);

        if (tt[len - 1] == '\n') {
            tt[len - 1] = 0;
        }

        if (tt[0] == '#' || strlen (tt) < 3) {
            continue;
        }
        break;
    }
}

static void cmd_arg (char **cmd, char **arg) {

    get_line ();
    char *s = tt;

    if (!*s) {
        *cmd = *arg = s;
        return;
    }

    s = skip_space (s);
    char *t = to_space (s);
    *cmd = s;
    if (!(*t)) {
        *arg = t;
        return;
    }

    *t = 0;
    t++;

    t = skip_space (t);
    del_newline_space (t);

    char *p = NULL;
    if ((p = strchr (t, '\t'))) {
        *p = 0;
    }

    *arg = t;
}

static int str_eq (const char *s, const char *t) {
    return (!strcmp (s, t));
}

typedef struct {
    u_int32_t key;
    uint8_t ch[CH_SZ];
    int oseq;
} ITEM2;

typedef struct {
    u_int64_t key;
    u_int8_t ch[CH_SZ];
    int oseq;
} ITEM2_64;

#define MAX_K (500000)

static ITEM2 itar[MAX_K];
static ITEM2_64 itar64[MAX_K];

static ITEM itout[MAX_K];
static ITEM64 itout64[MAX_K];

static int qcmp (const void *aa, const void *bb) {
    const ITEM2 *a = (ITEM2 *) aa;
    const ITEM2 *b = (ITEM2 *) bb;

    if (a->key > b->key) {
        return 1;
    }
    if (a->key < b->key) {
        return -1;
    }

    return a->oseq - b->oseq;
}

static int qcmp_64 (const void *aa, const void *bb) {
    ITEM2_64 *a = (ITEM2_64 *) aa;
    ITEM2_64 *b = (ITEM2_64 *) bb;

    if (a->key > b->key) {
        return 1;
    }
    if (a->key < b->key) {
        return -1;
    }

    return a->oseq - b->oseq;
}

#define mtolower(ch) ((ch) >= 'A' && (ch) <= 'Z' ? (ch) + 0x20 : (ch))

static char kno[128];

int main (int argc, char **argv) {

    printf ("-- hime-cin2gtab encoding UTF-8 --\n");
    printf ("--- please use iconv -f big5 -t utf-8 if your file is in big5 encoding\n");

    char fname[64];
    if (argc <= 1) {
        printf ("Enter table file name [.cin] : ");
        scanf ("%s", fname);
    } else {
        strncpy (fname, argv[1], sizeof (fname));
    }

    if (!strcmp (fname, "-v") || !strcmp (fname, "--version")) {
        p_err ("hime-cin2gtab for hime %s \n", HIME_VERSION);
        exit (0);
    }

    char *p = NULL;
    if ((p = strstr (fname, ".cin"))) {
        *p = 0;
    }

    char fname_cin[64];
    char fname_tab[64];
    strncpy (fname_cin, fname, sizeof (fname_cin));
    strncpy (fname_tab, fname, sizeof (fname_tab));
    strncat (fname_cin, ".cin", 4);
    strncat (fname_tab, ".gtab", 5);

    if ((fr = fopen (fname_cin, "rb")) == NULL) {
        p_err ("Cannot open %s\n", fname_cin);
    }

    skip_utf8_sigature (fr);

    struct TableHead th;
    char keymap[128];
    memset (&th, 0, sizeof (th));
    memset (kno, 0, sizeof (kno));
    memset (keymap, 0, sizeof (keymap));

    memset (itar, 0, sizeof (itar));
    memset (itout, 0, sizeof (itout));
    memset (itar64, 0, sizeof (itar64));
    memset (itout64, 0, sizeof (itout64));

    char *cmd = NULL;
    char *arg = NULL;
    cmd_arg (&cmd, &arg);
    if (str_eq (cmd, "%gen_inp")) {
        dbg ("skip gen_inp\n");
        cmd_arg (&cmd, &arg);
    }

    if (!str_eq (cmd, "%ename") || !(*arg)) {
        p_err ("%d:  %%ename english_name  expected", lineno);
    }
    arg[15] = 0;

    cmd_arg (&cmd, &arg);
    if (!(str_eq (cmd, "%prompt") || str_eq (cmd, "%cname")) || !(*arg)) {
        p_err ("%d:  %%prompt prompt_name  expected", lineno);
    }
    strncpy (th.cname, arg, MAX_CNAME);
    dbg ("cname %s\n", th.cname);

    cmd_arg (&cmd, &arg);
    if (!str_eq (cmd, "%selkey") || !(*arg)) {
        p_err ("%d:  %%selkey select_key_list expected", lineno);
    }

    if (strlen (arg) >= sizeof (th.selkey)) {
        memcpy (th.selkey, arg, sizeof (th.selkey));
        strcpy (th.selkey2, arg + sizeof (th.selkey));
        dbg ("th.selkey2 %s\n", th.selkey2);
    } else {
        strcpy (th.selkey, arg);
    }

    cmd_arg (&cmd, &arg);
    if (!str_eq (cmd, "%dupsel") || !(*arg)) {
        if (th.selkey[sizeof (th.selkey) - 1]) {
            th.M_DUP_SEL = sizeof (th.selkey) + strlen (th.selkey2);
        } else {
            th.M_DUP_SEL = strlen (th.selkey);
        }
    } else {
        th.M_DUP_SEL = atoi (arg);
        cmd_arg (&cmd, &arg);
    }

    for (;;) {
        if (str_eq (cmd, "%endkey")) {
            strcpy (th.endkey, arg);
            cmd_arg (&cmd, &arg);
        } else if (str_eq (cmd, "%space_style")) {
            th.space_style = (GTAB_space_pressed_E) atoi (arg);
            cmd_arg (&cmd, &arg);
        } else if (str_eq (cmd, "%keep_key_case")) {
            th.flag |= FLAG_KEEP_KEY_CASE;
            cmd_arg (&cmd, &arg);
        } else if (str_eq (cmd, "%symbol_kbm")) {
            th.flag |= FLAG_GTAB_SYM_KBM;
            cmd_arg (&cmd, &arg);
        } else if (str_eq (cmd, "%phase_auto_skip_endkey")) {
            th.flag |= FLAG_PHRASE_AUTO_SKIP_ENDKEY;
            cmd_arg (&cmd, &arg);
        } else if (str_eq (cmd, "%flag_auto_select_by_phrase")) {
            dbg ("flag_auto_select_by_phrase\n");
            th.flag |= FLAG_AUTO_SELECT_BY_PHRASE;
            cmd_arg (&cmd, &arg);
        } else if (str_eq (cmd, "%flag_disp_partial_match")) {
            dbg ("flag_disp_partial_match\n");
            th.flag |= FLAG_GTAB_DISP_PARTIAL_MATCH;
            cmd_arg (&cmd, &arg);
        } else if (str_eq (cmd, "%flag_disp_full_match")) {
            dbg ("flag_disp_full_match\n");
            th.flag |= FLAG_GTAB_DISP_FULL_MATCH;
            cmd_arg (&cmd, &arg);
        } else if (str_eq (cmd, "%flag_vertical_selection")) {
            dbg ("flag_vertical_selection\n");
            th.flag |= FLAG_GTAB_VERTICAL_SELECTION;
            cmd_arg (&cmd, &arg);
        } else if (str_eq (cmd, "%flag_press_full_auto_send")) {
            dbg ("flag_press_full_auto_send\n");
            th.flag |= FLAG_GTAB_PRESS_FULL_AUTO_SEND;
            cmd_arg (&cmd, &arg);
        } else if (str_eq (cmd, "%flag_unique_auto_send")) {
            dbg ("flag_unique_auto_send\n");
            th.flag |= FLAG_GTAB_UNIQUE_AUTO_SEND;
            cmd_arg (&cmd, &arg);
        } else {
            break;
        }
    }

    if (!str_eq (cmd, "%keyname") || !str_eq (arg, "begin")) {
        p_err ("%d:  %%keyname begin   expected, instead of %s %s", lineno, cmd, arg);
    }

    int KeyNum = 0;
    char kname[128][CH_SZ];
    for (KeyNum = 0;;) {
        char k = 0;

        cmd_arg (&cmd, &arg);
        if (str_eq (cmd, "%keyname")) {
            break;
        }
        if (BITON (th.flag, FLAG_KEEP_KEY_CASE)) {
            k = cmd[0];
        } else {
            k = mtolower (cmd[0]);
        }

        if (kno[(int) k]) {
            p_err ("%d:  key %c is already used", lineno, k);
        }

        kno[(int) k] = ++KeyNum;
        keymap[KeyNum] = k;
        bchcpy (&kname[KeyNum][0], arg);
    }

    keymap[0] = kname[0][0] = kname[0][1] = ' ';
    KeyNum++;
    th.KeyS = KeyNum; /* include space */

    cmd_arg (&cmd, &arg);

    if (str_eq (cmd, "%quick") && str_eq (arg, "begin")) {
        dbg (".. quick keys defined\n");
        for (int quick_def = 0;; quick_def++) {

            cmd_arg (&cmd, &arg);
            if (str_eq (cmd, "%quick")) {
                break;
            }

            const char k = kno[mtolower (cmd[0])] - 1;

            int N = 0;
            char *p = arg;

            if (strlen (cmd) == 1) {
                while (*p) {
                    int len = u8cpy (th.qkeys.quick1[(int) k][N++], p);
                    p += len;
                }
            } else if (strlen (cmd) == 2) {
                const int k1 = kno[mtolower (cmd[1])] - 1;
                while (*p) {
                    char tp[4];
                    int len = u8cpy (tp, p);

                    if (utf8_eq (tp, "□"))
                        tp[0] = 0;

                    u8cpy (th.qkeys.quick2[(int) k][(int) k1][N++], tp);
                    p += len;
                }
            } else {
                p_err ("%d:  %quick only 1&2 keys are allowed '%s'", lineno, cmd);
            }
        }
    }

    const long pos = ftell (fr);
    const int olineno = lineno;
    gboolean key64 = FALSE;
    int max_key_len = 0;

    while (!feof (fr)) {

        cmd_arg (&cmd, &arg);
        if (!cmd[0] || !arg[0])
            continue;

        if (!strcmp (cmd, "%chardef")) {
            if (!strcmp (arg, "end")) {
                break;
            } else {
                continue;
            }
        }

        int len = strlen (cmd);

        if (max_key_len < len) {
            max_key_len = len;
        }
    }

    fseek (fr, pos, SEEK_SET);
    lineno = olineno;

    INMD inmd, *cur_inmd = &inmd;

    cur_inmd->key64 = key64;
    cur_inmd->tbl64 = itout64;
    cur_inmd->tbl = itout;

    if (KeyNum < 64) {
        cur_inmd->keybits = 6;
    } else {
        cur_inmd->keybits = 7;
    }

    if (cur_inmd->keybits * max_key_len > 32) {
        cur_inmd->key64 = key64 = TRUE;
    }

    if (key64) {
        dbg ("key64\n");
    }

    printf ("KeyNum:%d keybits:%d\n", KeyNum, cur_inmd->keybits);

    th.keybits = cur_inmd->keybits;
    cur_inmd->last_k_bitn = (((cur_inmd->key64 ? 64 : 32) / cur_inmd->keybits) - 1) * cur_inmd->keybits;

    puts ("char def");
    int chno = 0;
    int *phridx = NULL;
    int phr_cou = 0;
    char *phrbuf = NULL;
    int prbf_cou = 0;
    while (!feof (fr)) {

        cmd_arg (&cmd, &arg);
        if (!cmd[0] || !arg[0])
            continue;

        if (!strcmp (cmd, "%chardef")) {
            if (!strcmp (arg, "end"))
                break;
            else
                continue;
        }

        int len = strlen (cmd);
        if (len > th.MaxPress) {
            th.MaxPress = len;
        }

        if (len > 10)
            p_err ("%d:  only <= 10 keys is allowed '%s'", lineno, cmd);

        u_int64_t kk = 0;
        for (int i = 0; i < len; i++) {
            int key = BITON (th.flag, FLAG_KEEP_KEY_CASE) ? cmd[i] : mtolower (cmd[i]);

            int k = kno[key];
            if (!k) {
                p_err ("%d: key undefined in keyname '%c'\n", lineno, cmd[i]);
            }

            kk |= (u_int64_t) k << (LAST_K_bitN - i * th.keybits);
        }

        //    dbg("%s kk:%llx\n", cmd, kk);

        if (key64) {
            memcpy (&itar64[chno].key, &kk, 8);
            itar64[chno].oseq = chno;
        } else {
            uint32_t key32 = (uint32_t) kk;
            memcpy (&itar[chno].key, &key32, 4);
            itar[chno].oseq = chno;
        }

        if ((len = strlen (arg)) <= CH_SZ && (arg[0] & 0x80)) {
            char out[CH_SZ + 1];

            memset (out, 0, sizeof (out));
            memcpy (out, arg, len);

            if (key64)
                bchcpy (itar64[chno].ch, out);
            else
                bchcpy (itar[chno].ch, out);

        } else {
            if (key64) {
                itar64[chno].ch[0] = phr_cou >> 16;
                itar64[chno].ch[1] = (phr_cou >> 8) & 0xff;
                itar64[chno].ch[2] = phr_cou & 0xff;
            } else {
                itar[chno].ch[0] = phr_cou >> 16;
                itar[chno].ch[1] = (phr_cou >> 8) & 0xff;
                itar[chno].ch[2] = phr_cou & 0xff;
            }

            if (len > MAX_CIN_PHR)
                p_err ("phrase too long: %s  max:%d bytes\n", arg, MAX_CIN_PHR);

            phridx = trealloc (phridx, int, phr_cou + 1);
            phridx[phr_cou++] = prbf_cou;
            phrbuf = (char *) realloc (phrbuf, prbf_cou + len + 1);
            strcpy (&phrbuf[prbf_cou], arg);
            //      printf("phrase:%d  len:%d'%s'\n", phr_cou, len, arg);
            prbf_cou += len;
        }

        chno++;
    }
    fclose (fr);

#define _sort qsort

    printf ("MaxPress: %d\n", th.MaxPress);

    th.DefC = chno;
    cur_inmd->DefChars = chno;

    if (key64)
        _sort (itar64, chno, sizeof (ITEM2_64), qcmp_64);
    else
        _sort (itar, chno, sizeof (ITEM2), qcmp);

    if (key64) {
        for (int i = 0; i < chno; i++) {
            memcpy (&itout64[i], &itar64[i], sizeof (ITEM64));
        }
    } else {
        for (int i = 0; i < chno; i++) {
            memcpy (&itout[i], &itar[i], sizeof (ITEM));
        }
    }

    char def1[256];
    gtab_idx1_t idx1[256];
    memset (def1, 0, sizeof (def1));
    memset (idx1, 0, sizeof (idx1));

    u_int64_t keymask = KEY_MASK;
    for (int i = 0; i < chno; i++) {
        u_int64_t key = CONVT2 (cur_inmd, i);
        int kk = (int) ((key >> LAST_K_bitN) & keymask);

        if (!def1[kk]) {
            idx1[kk] = (gtab_idx1_t) i;
            def1[kk] = 1;
        }
    }

    idx1[KeyNum] = chno;
    for (int i = KeyNum - 1; i > 0; i--) {
        if (!def1[i]) {
            idx1[i] = idx1[i + 1];
        }
    }

    if ((fw = fopen (fname_tab, "wb")) == NULL) {
        p_err ("Cannot create: %s", fname_tab);
        exit (1);
    }

    printf ("Defined Characters:%d\n", chno);

#if NEED_SWAP
    swap_byte_4 (&th.version);
    swap_byte_4 (&th.flag);
    swap_byte_4 (&th.space_style);
    swap_byte_4 (&th.KeyS);
    swap_byte_4 (&th.MaxPress);
    swap_byte_4 (&th.M_DUP_SEL);
    swap_byte_4 (&th.DefC);
    for (i = 0; i <= KeyNum; i++)
        swap_byte_4 (&idx1[i]);
#endif
    fwrite (&th, 1, sizeof (th), fw);
    fwrite (keymap, 1, KeyNum, fw);
    fwrite (kname, CH_SZ, KeyNum, fw);

    fwrite (idx1, sizeof (gtab_idx1_t), KeyNum + 1, fw);

    if (key64) {
#if NEED_SWAP
        for (i = 0; i < chno; i++) {
            swap_byte_8 (&itout64[i].key);
        }
#endif
        fwrite (itout64, sizeof (ITEM64), chno, fw);
#if 0
    for(i=0; i < 100; i++)
      dbg("%d] %c%c%c\n", i, itout64[i].ch[0], itout64[i].ch[1], itout64[i].ch[2]);
#endif
    } else {
#if NEED_SWAP
        for (i = 0; i < chno; i++) {
            swap_byte_4 (&itout[i].key);
        }
#endif
        fwrite (itout, sizeof (ITEM), chno, fw);
    }

    if (phr_cou) {
        phridx[phr_cou++] = prbf_cou;
        printf ("phrase count:%d\n", phr_cou);

        int ophr_cou = phr_cou;
#if NEED_SWAP
        for (i = 0; i < phr_cou; i++)
            swap_byte_4 (&phridx[i]);
        swap_byte_4 (&phr_cou);
#endif
        fwrite (&phr_cou, sizeof (int), 1, fw);
        fwrite (phridx, sizeof (int), ophr_cou, fw);
        fwrite (phrbuf, 1, prbf_cou, fw);
    }

    fclose (fw);

#if 0
  char bzip2[128];
  strcat(strcpy(bzip2, "bzip2 -f -k "), fname_tab);
  system(bzip2);
#endif

    return 0;
}
