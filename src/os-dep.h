#ifndef OS_DEP_H
#define OS_DEP_H

typedef char unich_t;
void unix_exec(char *fmt,...);

#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/keysym.h>

#include <glib.h>
#if GLIB_CHECK_VERSION(2,29,8)
#define G_CONST_RETURN const
#endif

#include <gdk/gdkx.h>

#endif
