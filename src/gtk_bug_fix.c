/**
 @file gtk_bug_fix.c
 @brief Workaround for gtk_window_resize() bug

 https://bugzilla.gnome.org/573123

*/

#include "os-dep.h" // for G_CONST_RETURN
#include <gtk/gtk.h>

void gdk_window_freeze_toplevel_updates_libgtk_only(GdkWindow *window) {}
void gdk_window_thaw_toplevel_updates_libgtk_only(GdkWindow *window) {}
