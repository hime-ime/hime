/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include "hime.h"
#include "hime-version.h"

static GtkWidget *about_window;

/* Our usual callback function */
static void callback_close( GtkWidget *widget, gpointer   data )
{
   gtk_widget_destroy(about_window);
   about_window = NULL;
}


void align_with_ui_window(GtkWidget *win);
int html_browser(char *fname);

void sys_icon_fname(char *iconame, char fname[]);
void create_about_window()
{
    if (about_window) {
      gtk_window_present(GTK_WINDOW(about_window));
      return;
    }

    GtkWidget *vbox = gtk_vbox_new(FALSE,3);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
    GtkWidget *hbox;

    /* Create a new about_window */
    about_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(about_window), GTK_WIN_POS_CENTER);

    gtk_window_set_has_resize_grip(GTK_WINDOW(about_window), FALSE);

    gtk_window_set_title (GTK_WINDOW (about_window), _(_L("關於 hime")));

    /* It's a good idea to do this for all windows. */
    g_signal_connect (G_OBJECT (about_window), "destroy",
	              G_CALLBACK (callback_close), NULL);

    g_signal_connect (G_OBJECT (about_window), "delete_event",
	 	      G_CALLBACK (callback_close), NULL);

    /* Sets the border width of the about_window. */
    gtk_container_set_border_width (GTK_CONTAINER (about_window), 10);


    GtkWidget *label_version;
    GtkWidget *image;

    /* Create box for image and label */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 3);

    GtkWidget *separator = gtk_hseparator_new ();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 3);

#if UNIX
    image = gtk_image_new_from_file (SYS_ICON_DIR"/hime.png");
#else
    char hime_png[128];
    sys_icon_fname("hime.png", hime_png);
    image = gtk_image_new_from_file (hime_png);
#endif

    label_version = gtk_label_new ("version " HIME_VERSION "  " __DATE__);

    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 3);
    gtk_box_pack_start (GTK_BOX (hbox), label_version, FALSE, FALSE, 3);


    gtk_container_add (GTK_CONTAINER (about_window), vbox);

    GtkWidget *button = gtk_button_new_with_label (_(_L("關閉")));
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 3);
    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (callback_close), (gpointer) "cool button");

    gtk_widget_show_all (about_window);

    return;
}
