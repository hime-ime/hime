/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

static gboolean timeout_destroy_window (GtkWidget *win) {
    gtk_widget_destroy (win);
    return FALSE;
}

#if TRAY_ENABLED
extern GtkStatusIcon *tray_icon;
extern GtkStatusIcon *icon_main;

extern gboolean is_exist_tray ();
extern gboolean is_exist_tray_double ();
#endif

static void create_win_message (char *icon, char *text, int duration) {
    GtkWidget *gwin_message = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_has_resize_grip (GTK_WINDOW (gwin_message), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (gwin_message), 0);
    gtk_widget_realize (gwin_message);
    set_no_focus (gwin_message);

    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (gwin_message), hbox);

    if (icon[0] != '-') {
        GtkWidget *image = gtk_image_new_from_file (icon);
        if (text[0] == '-') {
#if GTK_CHECK_VERSION(2, 91, 0)
            GdkPixbuf *pixbuf = NULL;
            GdkPixbufAnimation *anime = NULL;
            switch (gtk_image_get_storage_type (GTK_IMAGE (image))) {
            case GTK_IMAGE_PIXBUF:
                pixbuf = gtk_image_get_pixbuf (GTK_IMAGE (image));
                break;
            case GTK_IMAGE_ANIMATION:
                anime = gtk_image_get_animation (GTK_IMAGE (image));
                pixbuf = gdk_pixbuf_animation_get_static_image (anime);
                break;
            default:
                break;
            }
            cairo_surface_t *img = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, gdk_pixbuf_get_width (pixbuf), gdk_pixbuf_get_height (pixbuf));
            cairo_t *cr = cairo_create (img);
            gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
            cairo_paint (cr);
            cairo_region_t *mask = gdk_cairo_region_create_from_surface (img);
            gtk_widget_shape_combine_region (gwin_message, mask);
            cairo_region_destroy (mask);
            cairo_destroy (cr);
            cairo_surface_destroy (img);
#else
            GdkBitmap *bitmap = NULL;
            gdk_pixbuf_render_pixmap_and_mask (gdk_pixbuf_new_from_file (icon, NULL), NULL, &bitmap, 128);
            gtk_widget_shape_combine_mask (gwin_message, bitmap, 0, 0);
#endif
        }
        gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
    }

    if (text[0] != '-') {
        GtkWidget *label = gtk_label_new (text);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    }

    gtk_widget_show_all (gwin_message);

    int width, height;
    get_win_size (gwin_message, &width, &height);

    int ox = -1, oy;
#if TRAY_ENABLED
    GdkRectangle rect;
    GtkOrientation ori;
    if ((is_exist_tray () && gtk_status_icon_get_geometry (tray_icon, NULL, &rect, &ori)) || (is_exist_tray_double () && gtk_status_icon_get_geometry (icon_main, NULL, &rect, &ori))) {
        dbg ("rect %d,%d\n", rect.x, rect.y, rect.width, rect.height);
        if (ori == GTK_ORIENTATION_HORIZONTAL) {
            ox = rect.x;
            if (rect.y > 100)
                oy = rect.y - height;
            else
                oy = rect.y + rect.height;
        } else {
            oy = rect.y;
            if (rect.x > 100)
                ox = rect.x - width;
            else
                ox = rect.x + rect.width;
        }
    }
#endif
    if (ox < 0) {
        ox = dpy_xl - width;
        oy = dpy_yl - height;
    }

    gtk_window_move (GTK_WINDOW (gwin_message), ox, oy);

    g_timeout_add (duration, (GSourceFunc) timeout_destroy_window, gwin_message);
}

void execute_message (char *message) {
    char head[32];
    char icon[128];
    char text[128];
    int duration = 3000;

    icon[0] = text[0] = 0;

    sscanf (message, "%s %s %s %d", head, icon, text, &duration);

    create_win_message (icon, text, duration);
}
