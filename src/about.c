/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include "hime.h"

static GtkWidget *about_window;

/* Our usual callback function */
static void callback_close(GtkWidget *widget, gpointer dummy)
{
    gtk_widget_destroy(about_window);
    about_window = NULL;
}

void create_about_window()
{
    const gboolean no_expand = FALSE;
    const gboolean no_fill = FALSE;
    const guint padding = 3;

    if (about_window) {
        gtk_window_present(GTK_WINDOW (about_window));
        return;
    }

    /* Create a new about_window */
    about_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* Sets attributes of the about_window. */
    gtk_window_set_title(GTK_WINDOW (about_window), _("About hime"));
    gtk_container_set_border_width(GTK_CONTAINER (about_window), 10);
    gtk_window_set_position(GTK_WINDOW (about_window), GTK_WIN_POS_CENTER);

    /* It's a good idea to do this for all windows. */
    g_signal_connect(G_OBJECT (about_window), "destroy",
                     G_CALLBACK (callback_close), NULL);

    g_signal_connect(G_OBJECT (about_window), "delete_event",
                     G_CALLBACK (callback_close), NULL);


    /* Create box for icon image and label */
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER (hbox), 2);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 3);
    gtk_orientable_set_orientation(
        GTK_ORIENTABLE (vbox), GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX (vbox), hbox, no_expand, no_fill, padding);

    GtkWidget *separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX (vbox), separator, no_expand, no_fill, padding);


    /* hime icon image and label */
    GtkWidget *image = gtk_image_new_from_file(SYS_ICON_DIR "/hime.png");
    GtkWidget *label_version;
#if GIT_HAVE
    label_version = gtk_label_new("version " HIME_VERSION "\n(git " GIT_HASH ")");
#else
    label_version = gtk_label_new("version " HIME_VERSION);
#endif

    gtk_box_pack_start(GTK_BOX (hbox), image, no_expand, no_fill, padding);
    gtk_box_pack_start(GTK_BOX (hbox), label_version, no_expand, no_fill, padding);
    gtk_container_add(GTK_CONTAINER (about_window), vbox);

    /* close button */
    GtkWidget *button = gtk_button_new_with_label(_("Close"));
    gtk_box_pack_start(GTK_BOX (vbox), button, no_expand, no_fill, padding);
    g_signal_connect(G_OBJECT (button), "clicked",
                     G_CALLBACK (callback_close), NULL);

    gtk_widget_show_all(about_window);
    /* Put gtk_label_set_selectable() here so it will not be selected
     * by default. It is still selectable and can be copied.
     */
    gtk_label_set_selectable(GTK_LABEL (label_version), TRUE);

    return;
}
