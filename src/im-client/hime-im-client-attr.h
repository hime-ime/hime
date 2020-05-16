/*
 * Copyright (C) 2020 The HIME team, Taiwan
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

#ifndef HIME_IM_CLIENT_ATTR_H
#define HIME_IM_CLIENT_ATTR_H

#define HIME_PREEDIT_ATTR_FLAG_UNDERLINE 1
#define HIME_PREEDIT_ATTR_FLAG_REVERSE 2
#define HIME_PREEDIT_ATTR_MAX_N 64
#define HIME_PREEDIT_MAX_STR 512

typedef struct {
    int flag;
    short ofs0, ofs1;  // ofs : bytes offset
} HIME_PREEDIT_ATTR;

#endif /* HIME_IM_CLIENT_ATTR_H */
