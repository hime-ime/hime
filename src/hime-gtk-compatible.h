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

// version check
// GTK+3
#if GTK_CHECK_VERSION(3, 0, 0) && !GTK_CHECK_VERSION(3, 22, 30)
#error GTK+ 3.22.30 is required
#endif

// GTK+2
#if !GTK_CHECK_VERSION(2, 24, 31)
#error GTK+ 2.24.31 is required
#endif

#ifndef HIME_GTK_COMPATIBLE_H
#define HIME_GTK_COMPATIBLE_H

// compat macro for Gtk+2/Gtk+3
#undef GDK_DISPLAY
#define GDK_DISPLAY() GDK_DISPLAY_XDISPLAY (gdk_display_get_default ())

// only in Gtk+2, compat for Gtk+3
#if GTK_CHECK_VERSION(3, 0, 0)
#define gtk_hseparator_new() gtk_separator_new (GTK_ORIENTATION_HORIZONTAL)
#define gtk_vseparator_new() gtk_separator_new (GTK_ORIENTATION_VERTICAL)
#define GTK_OBJECT

#define gtk_hbox_new(x, y) gtk_grid_new ()
#define gtk_vbox_new(x, y) gtk_grid_new ()
#define gtk_box_pack_end(v, w, x, y, z) gtk_container_add (GTK_CONTAINER (v), w)
#define gtk_box_pack_start(v, w, x, y, z) gtk_container_add (GTK_CONTAINER (v), w)

#define GDK_WINDOW_XWINDOW GDK_WINDOW_XID
#endif

// only in Gtk+3, compat for Gtk+2
#if !GTK_CHECK_VERSION(3, 0, 0)
#define gtk_widget_get_preferred_size(x, y, z) gtk_widget_size_request (x, z)
#define gtk_widget_set_halign(x, y) ;

#define gtk_window_set_has_resize_grip(x, y) ;
#define gtk_widget_set_hexpand(x, y) ;
#define gtk_widget_set_vexpand(x, y) ;

#define gtk_grid_set_column_homogeneous(x, y) ;
#define gtk_grid_set_row_homogeneous(x, y) ;

#define gtk_orientable_set_orientation(x, y) ;
#endif

// XXX(xatier): both gtk_widget_modify_font and gtk_widget_override_font are deprecated
#ifndef gtk_widget_override_font
#define gtk_widget_override_font gtk_widget_modify_font
#endif

#if GTK_CHECK_VERSION(3, 3, 2)
#undef GTK_TABLE
#define GTK_TABLE GTK_GRID
#define gtk_table_attach_defaults(u, v, w, x, y, z) gtk_grid_attach (u, v, w, y, 1, 1)
#define gtk_table_new(x, y, z) gtk_grid_new ()
#endif

#if GTK_CHECK_VERSION(3, 3, 18)
#define GTK_COLOR_SELECTION_DIALOG GTK_COLOR_CHOOSER_DIALOG
#define GTK_COLOR_SELECTION GTK_COLOR_CHOOSER
#endif

#if GTK_CHECK_VERSION(3, 9, 10)
#define gtk_button_new_from_stock(x) gtk_button_new_from_icon_name (x, GTK_ICON_SIZE_BUTTON)
#define GTK_STOCK_CANCEL "gtk-cancel"
#define GTK_STOCK_OK "gtk-ok"
#define GTK_STOCK_QUIT "gtk-quit"
#define GTK_STOCK_SAVE "gtk-save"
#define GTK_STOCK_OPEN "gtk-open"
#define GTK_STOCK_CLOSE "gtk-close"
#define GTK_STOCK_DELETE "gtk-delete"
#define GTK_STOCK_FIND "gtk-find"
#endif

#if GTK_CHECK_VERSION(3, 13, 4)
#define gtk_window_set_has_resize_grip(x, y) ;
#endif

#ifndef PANGO_VERSION_CHECK
#define PANGO_VERSION_CHECK(x, y, z) FALSE
#endif

#endif /* HIME_GTK_COMPATIBLE_H */
