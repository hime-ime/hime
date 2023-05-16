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

#ifndef OS_DEP_H
#define OS_DEP_H

typedef char unich_t;
void unix_exec (char *fmt, ...);

#include <glib.h>

#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/keysym.h>
#include <gdk/gdkx.h>

#endif /* HIME_OS_DEP_H */
