/* Copyright (C) 2009 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#define swap_ch(a, b) do { char t; t = *(a); *(a) = *(b); *(b) = t; } while (0)

#define swap_byte_2(pp) do { char *p=(char *)pp;  swap_ch(p, p+1); } while (0)
#define swap_byte_4(pp) do { char *p=(char *)pp;  swap_ch(p, p+3); swap_ch(p+1, p+2); } while (0)
#define swap_byte_8(pp) do { char *p=(char *)pp;  swap_ch(p, p+7); swap_ch(p+1, p+6); swap_ch(p+2, p+5); swap_ch(p+3, p+4);} while (0)

#if __BYTE_ORDER == __BIG_ENDIAN
//#warning "big endian"
#define to_hime_endian_2(pp) swap_byte_2(pp)
#define to_hime_endian_4(pp) swap_byte_4(pp)
#define to_hime_endian_8(pp) swap_byte_8(pp)
#else
#define to_hime_endian_2(pp) do { } while (0)
#define to_hime_endian_4(pp) do { } while (0)
#define to_hime_endian_8(pp) do { } while (0)
#endif
