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

#ifndef GST_H
#define GST_H

#include <X11/Xlib.h>
#include <gtk/gtk.h>

typedef struct {
    struct CHPHO *chpho;
    int c_idx;         ///< the current cursor position
    int c_len;         ///< the length of the preedit buffer
    int ph_sta;        // phrase start
    int sel_pho;       ///< indicating the selection window is being used or not
    int current_page;  ///< the current index among all the selection candidates
    int startf;
    gboolean full_match;
    gboolean tsin_buffer_editing;  ///< indicating a special mode of preedit buffer editing
    gboolean ctrl_pre_sel;
    struct PRE_SEL *pre_sel;
    int pre_selN;
    int last_cursor_idx;  ///< the last cursor position
    int pho_menu_idx;     ///< the current selected index of the candidates in a single selection window menu
} TSIN_ST;
extern TSIN_ST tss;

typedef enum {
    SAME_PHO_QUERY_none = 0,
    SAME_PHO_QUERY_gtab_input = 1,
    SAME_PHO_QUERY_pho_select = 2,
} SAME_PHO_QUERY;

typedef struct {
    gboolean ityp3_pho;
    int cpg, maxi;
    int start_idx, stop_idx;
    char typ_pho[4];
    char inph[8];
    SAME_PHO_QUERY same_pho_query_state;
} PHO_ST;
extern PHO_ST poo;

#define MAX_TAB_KEY_NUM64_6 (10)

typedef struct {
    int S1, E1, last_idx, wild_page, pg_idx, total_matchN, sel1st_i;
    u_int64_t kval;
    gboolean last_full, wild_mode, spc_pressed, invalid_spc, more_pg, gtab_buf_select;
    short defselN, exa_match, ci, gbufN, gbuf_cursor;
    KeySym inch[MAX_TAB_KEY_NUM64_6];
} GTAB_ST;
extern GTAB_ST ggg;

#endif /* HIME_GST_H */
