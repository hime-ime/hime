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

#ifndef WIN0_H
#define WIN0_H

void init_win0 (void);
void destroy_win0 (void);
void show_win0 (void);
void hide_win0 (void);
void show_button_pho ();
void hide_button_pho ();
gboolean is_win0_visible (void);
void get_win0_geom ();
void move_win0_auto (void);
void move_win0 (int x, int y);
void reset_content (void);
void set_cursor (int index);
void clear_cursor (int index);
void clear_and_hide_chars_all (void);
void clear_and_hide_char (int index);
void clear_phonemes ();

#endif
