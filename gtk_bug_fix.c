#include "os-dep.h"
#include <gtk/gtk.h>

void gdk_window_freeze_toplevel_updates_libgtk_only(GdkWindow *window) {}
void gdk_window_thaw_toplevel_updates_libgtk_only(GdkWindow *window) {}
