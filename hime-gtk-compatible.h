#if !GTK_CHECK_VERSION(2,4,0)
#define GTK_COMBO_BOX GTK_OPTION_MENU
#define gtk_combo_box_get_active gtk_option_menu_get_history
#define gtk_combo_box_set_active gtk_option_menu_set_history
#define gtk_combo_box_new_text gtk_option_menu_new
#endif

#if !GTK_CHECK_VERSION(2,13,4)
#define gtk_widget_get_window(x) (x)->window
#define gtk_color_selection_dialog_get_color_selection(x) (x)->colorsel
#endif

#if !GTK_CHECK_VERSION(2,15,0)
#define gtk_status_icon_set_tooltip_text gtk_status_icon_set_tooltip
#endif

#if GTK_CHECK_VERSION(2,17,5)
#undef GTK_WIDGET_NO_WINDOW
#define GTK_WIDGET_NO_WINDOW !gtk_widget_get_has_window
#undef GTK_WIDGET_SET_FLAGS
#define GTK_WIDGET_SET_FLAGS(x,y) gtk_widget_set_can_default(x,1)
#endif

#if GTK_CHECK_VERSION(2,17,7)
#undef GTK_WIDGET_VISIBLE
#define GTK_WIDGET_VISIBLE gtk_widget_get_visible
#endif

#if GTK_CHECK_VERSION(2,17,10)
#undef GTK_WIDGET_DRAWABLE
#define GTK_WIDGET_DRAWABLE gtk_widget_is_drawable
#endif

#if GTK_CHECK_VERSION(2,19,5)
#undef GTK_WIDGET_REALIZED
#define GTK_WIDGET_REALIZED gtk_widget_get_realized
#endif

#if GTK_CHECK_VERSION(2,21,8)
#undef GDK_DISPLAY
#define GDK_DISPLAY() GDK_DISPLAY_XDISPLAY(gdk_display_get_default())
#endif

#if GTK_CHECK_VERSION(2,24,0)
#define gtk_combo_box_new_text gtk_combo_box_text_new
#define gtk_combo_box_append_text gtk_combo_box_text_append_text
#define gtk_widget_hide_all gtk_widget_hide
#endif

#if GTK_CHECK_VERSION(2,90,0)
#undef GTK_CHECK_CAST
#define GTK_CHECK_CAST G_TYPE_CHECK_INSTANCE_CAST
#undef GDK_DRAWABLE_XID
#define GDK_DRAWABLE_XID GDK_WINDOW_XID
#undef GDK_DRAWABLE_XDISPLAY
#define GDK_DRAWABLE_XDISPLAY GDK_WINDOW_XDISPLAY
#define gtk_hseparator_new() gtk_separator_new(GTK_ORIENTATION_HORIZONTAL)
#define gtk_vseparator_new() gtk_separator_new(GTK_ORIENTATION_VERTICAL)
#endif

#if !GTK_CHECK_VERSION(2,91,0)
#define gdk_error_trap_pop_ignored gdk_error_trap_pop
#define gtk_widget_get_preferred_size(x,y,z) gtk_widget_size_request(x,z)
#define gtk_widget_set_halign(x,y);
#endif

#if GTK_CHECK_VERSION(2,91,0)
#define GTK_OBJECT
#define gdk_drawable_get_screen gdk_window_get_screen
#endif

#if !GTK_CHECK_VERSION(2,91,1)
#define gtk_window_set_has_resize_grip(x,y);
#define gtk_widget_set_hexpand(x,y);
#define gtk_widget_set_vexpand(x,y);
#endif

#if !GTK_CHECK_VERSION(2,91,2)
#define gtk_grid_set_column_homogeneous(x,y);
#define gtk_grid_set_row_homogeneous(x,y);
#define gtk_orientable_set_orientation(x,y);
#endif

#if GTK_CHECK_VERSION(2,91,2)
#undef GTK_BOX
#define GTK_BOX GTK_GRID
#define gtk_hbox_new(x,y) gtk_grid_new()
#define gtk_vbox_new(x,y) gtk_grid_new()
#define gtk_box_pack_end(v,w,x,y,z) gtk_container_add(GTK_CONTAINER(v),w)
#define gtk_box_pack_start(v,w,x,y,z) gtk_container_add(GTK_CONTAINER(v),w)
#endif

#ifndef GTK_COMBO_BOX_TEXT
#define GTK_COMBO_BOX_TEXT GTK_COMBO_BOX
#endif

#if !GTK_CHECK_VERSION(2,91,6)
#define gtk_widget_override_font gtk_widget_modify_font
#endif

#if GTK_CHECK_VERSION(2,91,6)
#define GDK_WINDOW_XWINDOW GDK_WINDOW_XID
#endif

#if GTK_CHECK_VERSION(3,3,2)
#undef      GTK_TABLE
#define     GTK_TABLE GTK_GRID
#define     gtk_table_attach_defaults(u,v,w,x,y,z) gtk_grid_attach(u,v,w,y,1,1)
#define     gtk_table_new(x,y,z) gtk_grid_new()
#endif
