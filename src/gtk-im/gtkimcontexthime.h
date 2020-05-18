/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * GTK - The GIMP Toolkit
 * Copyright (C) 2000 Red Hat Software
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

#ifndef HIME_GTKIMCONTEXTHIME_H
#define HIME_GTKIMCONTEXTHIME_H

#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>
#if !GTK_CHECK_VERSION(3, 0, 0)
#include <gdk/gdkkeysyms.h>
#endif

#include "../hime-gtk-compatible.h"

extern GType gtk_type_im_context_hime;

#define GTK_TYPE_IM_CONTEXT_HIME gtk_type_im_context_hime
#define GTK_IM_CONTEXT_HIME(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_IM_CONTEXT_HIME, GtkIMContextHIME))
#define GTK_IM_CONTEXT_HIME_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_IM_CONTEXT_HIME, GtkIMContextHIME const))
#define GTK_IM_CONTEXT_HIME_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_IM_CONTEXT_HIME, GtkIMContextHIMEClass))
#define GTK_IS_IM_CONTEXT_HIME(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_IM_CONTEXT_HIME))
#define GTK_IS_IM_CONTEXT_HIME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_IM_CONTEXT_HIME))
#define GTK_IM_CONTEXT_HIME_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_IM_CONTEXT_HIME, GtkIMContextHIMEClass))

typedef struct _GtkIMContextHIME GtkIMContextHIME;
typedef struct _GtkIMContextHIMEClass GtkIMContextHIMEClass;

struct _GtkIMContextHIMEClass {
    GtkIMContextClass parent_class;
};

void gtk_im_context_hime_register_type (GTypeModule *type_module);

GtkIMContext *gtk_im_context_hime_new (void);

void gtk_im_context_hime_shutdown (void);

#endif /* HIME_GTKIMCONTEXTHIME_H */
