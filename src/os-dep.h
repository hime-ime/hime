#ifndef OS_DEP_H
#define OS_DEP_H

typedef char unich_t;
void unix_exec(char *fmt,...);

#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/keysym.h>

#include <glib.h>

#include <gdk/gdkx.h>

#endif
