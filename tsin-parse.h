/* Copyright (C) 2010 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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
  char len, flag;
  u_char start;
  unsigned char str[MAX_PHRASE_LEN*CH_SZ+1];  // use malloc
} TSIN_PARSE;

enum {
  FLAG_TSIN_PARSE_PHRASE = 1,
  FLAG_TSIN_PARSE_PARTIAL = 2, //partial phrase
};

typedef struct {
  int start;
  int usecount;
  short match_phr_N, no_match_ch_N;
  TSIN_PARSE best[MAX_PH_BF_EXT+1];
} CACHE;

void tsin_parse();
void init_cache(int tc_len);
CACHE *cache_lookup(int start);
int tsin_parse_recur(int start, TSIN_PARSE *out,
                     short *r_match_phr_N, short *r_no_match_ch_N);
