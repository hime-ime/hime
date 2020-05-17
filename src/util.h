/*
 * Copyright (C) 2020 The HIME team, Taiwan
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

#ifndef HIME_UTIL_H
#define HIME_UTIL_H

void p_err (char *fmt, ...);

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
void __hime_dbg_ (char *fmt, ...);
#define dbg(fmt, ...) __hime_dbg_ ("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define dbg(...) \
    do {         \
    } while (0)
#endif

#endif /* HIME_UTIL_H */
