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

#ifndef WIN_PHO_H
#define WIN_PHO_H

void init_win_pho (void);
void destroy_win_pho (void);
void show_win_pho (void);
void hide_win_pho (void);
gboolean is_win_pho_visible (void);
void get_win_pho_geom (void);
void move_win_pho (int x, int y);
void set_phoneme_at_index (int index, char *phochar);
void win_pho_disp_half_full (void);

#endif
