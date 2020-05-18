/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * GTK - The GIMP Toolkit
 * Copyright (C) 2000 Red Hat, Inc.
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

#include <string.h>

#include "gtkimcontexthime.h"
#include "gtkintl.h"

static const GtkIMContextInfo hime_info = {
    "hime",                   /* unique identification string */
    N_ ("hime Input Method"), /* human-readable name */
    GETTEXT_PACKAGE,          /* Translation domain to be used with dgettext() */
    GTK_LOCALEDIR,            /* Name of locale directory for use with bindtextdomain() */
    "zh:ja"                   /* A colon-separated list of locales where this input method should
                                 be the default. */
};

static const GtkIMContextInfo *info_list[] = {
    &hime_info};

void im_module_init (GTypeModule *type_module) {
    gtk_im_context_hime_register_type (type_module);
}

void im_module_exit (void) {
    gtk_im_context_hime_shutdown ();
}

void im_module_list (const GtkIMContextInfo ***contexts,
                     int *n_contexts) {
    *contexts = info_list;
    *n_contexts = G_N_ELEMENTS (info_list);
}

GtkIMContext *im_module_create (const gchar *context_id) {
    if (strcmp (context_id, "hime") == 0) {
        return gtk_im_context_hime_new ();
    }

    return NULL;
}
