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

#ifndef HIME_GTKINTL_H
#define HIME_GTKINTL_H

/* To get GETTEXT_PACKAGE */
#include "../config.h"

/* TODO: Should support build-time configuration */
#define GTK_LOCALEDIR "/usr/share/locale"

#if HIME_I18N_MESSAGE
#include <libintl.h>
#define _(String) dgettext (GETTEXT_PACKAGE, String)
#else
#define _(String) (String)
#endif
#define N_(String) (String)

#endif /* HIME_GTKINTL_H */
