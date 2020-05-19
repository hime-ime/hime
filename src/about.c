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
static void callback_close (GtkWidget *widget, gpointer dummy) {
    gtk_widget_destroy (about_window);
    about_window = NULL;
}

/* Create a new about_window */
static GtkWidget *get_new_about_window (void) {
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    /* Sets attributes of the about_window. */
    gtk_window_set_title (GTK_WINDOW (window), _ ("About hime"));
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

    /* kill signals */
    g_signal_connect (
        G_OBJECT (window), "destroy",
        G_CALLBACK (callback_close), NULL);

    g_signal_connect (
        G_OBJECT (window), "delete-event",
        G_CALLBACK (callback_close), NULL);

    return window;
}

/*
 * Create a new hbox
 *
 * This is the container for the HIME icon image and the version label.
 */
static GtkWidget *get_new_hbox (void) {
    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);

    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

    return hbox;
}

/*
 * Create a new vbox
 *
 * This is the container for everthing under the about window.
 */
static GtkWidget *get_new_vbox (void) {
    GtkWidget *vbox = gtk_vbox_new (FALSE, 3);
    gtk_orientable_set_orientation (
        GTK_ORIENTABLE (vbox), GTK_ORIENTATION_VERTICAL);

    return vbox;
}

/*
 * Create a new label for HIME_VERSION (with GIT_HASH)
 */
static GtkWidget *get_version_label (void) {
    GtkWidget *label;

#if GIT_HAVE
    label = gtk_label_new ("version " HIME_VERSION "\n(git " GIT_HASH ")");
#else
    label = gtk_label_new ("version " HIME_VERSION);
#endif

    return label;
}

/*
 * Create a new gtk button for close button
 */
static GtkWidget *get_close_button (void) {
    GtkWidget *button = gtk_button_new_with_label (_ ("Close"));
    g_signal_connect (
        G_OBJECT (button), "clicked",
        G_CALLBACK (callback_close), NULL);

    return button;
}

/* Put a child Widget inside the parent box */
static void box_add (GtkBox *parent, GtkWidget *child) {
    /* no expand, no filling, padding = 3 */
    gtk_box_pack_start (parent, child, FALSE, FALSE, 3);
}

void create_about_window () {

    if (about_window) {
        gtk_window_present (GTK_WINDOW (about_window));
        return;
    }

    about_window = get_new_about_window ();

    GtkWidget *hbox = get_new_hbox ();
    GtkWidget *vbox = get_new_vbox ();
    GtkWidget *separator = gtk_hseparator_new ();
    GtkWidget *image = gtk_image_new_from_file (SYS_ICON_DIR "/hime.png");
    GtkWidget *version_label = get_version_label ();
    GtkWidget *close_button = get_close_button ();

    box_add (GTK_BOX (vbox), hbox);
    box_add (GTK_BOX (vbox), separator);
    box_add (GTK_BOX (hbox), image);
    box_add (GTK_BOX (hbox), version_label);
    gtk_container_add (GTK_CONTAINER (about_window), vbox);
    box_add (GTK_BOX (vbox), close_button);

    gtk_widget_show_all (about_window);

    /* Put gtk_label_set_selectable() here so it will not be selected
     * by default. It is still selectable and can be copied.
     */
    gtk_label_set_selectable (GTK_LABEL (version_label), TRUE);

    return;
}
