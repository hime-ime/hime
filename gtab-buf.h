/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

typedef struct {
  char *ch;
  char **sel;
  int selN;
  u_short flag;
  u_char c_sel;
  char plen, keysN;
  u_int64_t keys[8];
} GEDIT;

extern GEDIT *gbuf;
extern short gbufN;

void insert_gbuf_nokey(char *s);
void insert_gbuf_cursor1_cond(char *s, u_int64_t key, gboolean valid_key);
GEDIT *insert_gbuf_cursor(char **sel, int selN, u_int64_t key, gboolean b_gtab_en_no_spc);
