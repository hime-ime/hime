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
#define GTK_OBJECT

#define GtkStatusIcon GObject
#define gtk_status_icon_position_menu NULL

#define GTK_COLOR_SELECTION
#define GTK_COLOR_SELECTION_DIALOG

#define GDK_WINDOW_XWINDOW GDK_WINDOW_XID
#endif

// only in Gtk+3, compat for Gtk+2
#if !GTK_CHECK_VERSION(3, 0, 0)
#define gtk_separator_new(orientation) (orientation == GTK_ORIENTATION_HORIZONTAL ? gtk_hseparator_new () : gtk_vseparator_new ())

#define gtk_box_new(orientation, y) (orientation == GTK_ORIENTATION_HORIZONTAL ? gtk_hbox_new (FALSE, y) : gtk_vbox_new (FALSE, y))

#define gtk_widget_get_preferred_size(x, y, z) gtk_widget_size_request (x, z)
#define gtk_widget_set_halign(x, y) ;

#define gtk_window_set_has_resize_grip(x, y) ;
#define gtk_widget_set_hexpand(x, y) ;
#define gtk_widget_set_vexpand(x, y) ;

#define gtk_grid_set_column_homogeneous(x, y) ;
#define gtk_grid_set_row_homogeneous(x, y) ;

#define gtk_orientable_set_orientation(x, y) ;

#define GdkRGBA GdkColor
#define gdk_rgba_parse(rgba, spec) gdk_color_parse (spec, rgba)
#define gdk_rgba_to_string(rgba) gdk_color_to_string (rgba)
#define gtk_color_selection_get_current_rgba(colorsel, rgba) gtk_color_selection_get_current_color (colorsel, rgba)
#define gtk_color_selection_set_current_rgba(colorsel, rgba) gtk_color_selection_set_current_color (colorsel, rgba)
#define gtk_widget_override_color(widget, state, color) gtk_widget_modify_fg (widget, state, color)
#define GTK_STATE_FLAG_NORMAL GTK_STATE_NORMAL
#define GTK_STATE_FLAG_ACTIVE GTK_STATE_ACTIVE
#define GTK_STATE_FLAG_PRELIGHT GTK_STATE_PRELIGHT
#define GTK_STATE_FLAG_SELECTED GTK_STATE_SELECTED
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
#define GTK_STOCK_ABOUT "gtk-about"
#define GTK_STOCK_PREFERENCES "gtk-preferences"
#define GTK_STOCK_INDEX "gtk-index"
#endif

#if GTK_CHECK_VERSION(3, 13, 4)
#define gtk_window_set_has_resize_grip(x, y) ;
#endif

#ifndef PANGO_VERSION_CHECK
#define PANGO_VERSION_CHECK(x, y, z) FALSE
#endif

#endif /* HIME_GTK_COMPATIBLE_H */
