/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 2009 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include "hime-protocol.h"

static uint32_t __hime_rand__ (uint32_t *next) {
    *next = *next * 1103515245 + 12345;
    return (*next / 65536) % 32768;
}

void __hime_enc_mem (unsigned char *p,
                     const int n,
                     const HIME_PASSWD *passwd,
                     uint32_t *seed) {
    for (int i = 0; i < n; i++) {
        uint32_t v = __hime_rand__ (seed) % __HIME_PASSWD_N_;
        p[i] ^= passwd->passwd[v];
    }
}
