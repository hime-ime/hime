/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* eggtrayicon.c
 * Copyright (C) 2002 Anders Carlsson <andersca@gnu.org>
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

#include <string.h>

#include "eggtrayicon.h"

#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include "hime.h"

#if GTK_CHECK_VERSION(2,24,0)
#define gdk_window_lookup_for_display gdk_x11_window_lookup_for_display
#endif


#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

#define SYSTEM_TRAY_ORIENTATION_HORZ 0
#define SYSTEM_TRAY_ORIENTATION_VERT 1

enum {
  PROP_0,
  PROP_ORIENTATION
};

static GtkPlugClass *parent_class = NULL;

static void egg_tray_icon_init (EggTrayIcon *icon);
static void egg_tray_icon_class_init (EggTrayIconClass *klass);

static void egg_tray_icon_get_property (GObject    *object,
					guint       prop_id,
					GValue     *value,
					GParamSpec *pspec);

static void egg_tray_icon_realize   (GtkWidget *widget);
static void egg_tray_icon_unrealize (GtkWidget *widget);

static void egg_tray_icon_add (GtkContainer *container,
                               GtkWidget *widget);

static void egg_tray_icon_update_manager_window    (EggTrayIcon *icon,
						    gboolean     dock_if_realized);
static void egg_tray_icon_manager_window_destroyed (EggTrayIcon *icon);

GType
egg_tray_icon_get_type (void)
{
  static GType our_type = 0;

  if (our_type == 0)
    {
      our_type = g_type_from_name("EggTrayIcon");

      if (our_type == 0)
        {
      static const GTypeInfo our_info =
      {
	sizeof (EggTrayIconClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) egg_tray_icon_class_init,
	NULL, /* class_finalize */
	NULL, /* class_data */
	sizeof (EggTrayIcon),
	0,    /* n_preallocs */
	(GInstanceInitFunc) egg_tray_icon_init,
	NULL /* value_table */
      };

      our_type = g_type_register_static (GTK_TYPE_PLUG, "EggTrayIcon", &our_info, (GTypeFlags)0);
        }
      else if (parent_class == NULL)
        {
          /* we're reheating the old class from a previous instance -  engage ugly hack =( */
          egg_tray_icon_class_init((EggTrayIconClass *)g_type_class_peek(our_type));
        }
    }

  return our_type;
}

static void
egg_tray_icon_init (EggTrayIcon *icon)
{
  icon->stamp = 1;
  icon->orientation = GTK_ORIENTATION_HORIZONTAL;

  gtk_widget_add_events (GTK_WIDGET (icon), GDK_PROPERTY_CHANGE_MASK);
}

static void
egg_tray_icon_class_init (EggTrayIconClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;
  GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
  GtkContainerClass *container_class = (GtkContainerClass *)klass;

  parent_class = (GtkPlugClass*)g_type_class_peek_parent (klass);

  gobject_class->get_property = egg_tray_icon_get_property;

  widget_class->realize   = egg_tray_icon_realize;
  widget_class->unrealize = egg_tray_icon_unrealize;

  container_class->add = egg_tray_icon_add;

  g_object_class_install_property (gobject_class,
				   PROP_ORIENTATION,
				   g_param_spec_enum ("orientation",
						      _("Orientation"),
						      _("The orientation of the tray."),
						      GTK_TYPE_ORIENTATION,
						      GTK_ORIENTATION_HORIZONTAL,
						      G_PARAM_READABLE));
}

static void
egg_tray_icon_get_property (GObject    *object,
			    guint       prop_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
  EggTrayIcon *icon = EGG_TRAY_ICON (object);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, icon->orientation);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
egg_tray_icon_get_orientation_property (EggTrayIcon *icon)
{
  Display *xdisplay;
  Atom type;
  int format;
  union {
	gulong *prop;
	guchar *prop_ch;
  } prop = { NULL };
  gulong nitems;
  gulong bytes_after;
  int error, result;

  g_return_if_fail(icon->manager_window != None);

#if GTK_CHECK_VERSION(2,1,0)
  xdisplay = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));
#else
  xdisplay = gdk_display;
#endif

  gdk_error_trap_push ();
  type = None;
  result = XGetWindowProperty (xdisplay,
			       icon->manager_window,
			       icon->orientation_atom,
			       0, G_MAXLONG, FALSE,
			       XA_CARDINAL,
			       &type, &format, &nitems,
			       &bytes_after, &(prop.prop_ch));
  error = gdk_error_trap_pop ();

  if (error || result != Success)
    return;

  if (type == XA_CARDINAL)
    {
      GtkOrientation orientation;

      orientation = (prop.prop [0] == SYSTEM_TRAY_ORIENTATION_HORZ) ?
					GTK_ORIENTATION_HORIZONTAL :
					GTK_ORIENTATION_VERTICAL;

      if (icon->orientation != orientation)
	{
	  icon->orientation = orientation;

	  g_object_notify (G_OBJECT (icon), "orientation");
	}
    }

  if (prop.prop)
    XFree (prop.prop);
}

static GdkFilterReturn
egg_tray_icon_manager_filter (GdkXEvent *xevent, GdkEvent *event, gpointer user_data)
{
  EggTrayIcon *icon = (EggTrayIcon*)user_data;
  XEvent *xev = (XEvent *)xevent;

  if (xev->xany.type == ClientMessage &&
      xev->xclient.message_type == icon->manager_atom &&
      xev->xclient.data.l[1] == icon->selection_atom)
    {
      egg_tray_icon_update_manager_window (icon, TRUE);
    }
  else if (xev->xany.window == icon->manager_window)
    {
      if (xev->xany.type == PropertyNotify &&
	  xev->xproperty.atom == icon->orientation_atom)
	{
	  egg_tray_icon_get_orientation_property (icon);
	}
      if (xev->xany.type == DestroyNotify)
	{
	  egg_tray_icon_manager_window_destroyed (icon);
	}
    }

  return GDK_FILTER_CONTINUE;
}

static void
egg_tray_icon_unrealize (GtkWidget *widget)
{
  EggTrayIcon *icon = EGG_TRAY_ICON (widget);
  GdkWindow *root_window;

  if (icon->manager_window != None)
    {
      GdkWindow *gdkwin;

#if GTK_CHECK_VERSION(2,1,0)
      gdkwin = gdk_window_lookup_for_display (gtk_widget_get_display (widget),
                                              icon->manager_window);
#else
      gdkwin = gdk_window_lookup (icon->manager_window);
#endif

      gdk_window_remove_filter (gdkwin, egg_tray_icon_manager_filter, icon);
    }

#if GTK_CHECK_VERSION(2,1,0)
  GdkScreen *screen = gtk_widget_get_screen (widget);
  if (screen)
    root_window = gdk_screen_get_root_window (screen);
  else
    return;
#else
  root_window = gdk_window_lookup (gdk_x11_get_default_root_xwindow ());
#endif

  gdk_window_remove_filter (root_window, egg_tray_icon_manager_filter, icon);

  if (GTK_WIDGET_CLASS (parent_class)->unrealize)
    (* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static void
egg_tray_icon_send_manager_message (EggTrayIcon *icon,
				    long         message,
				    Window       window,
				    long         data1,
				    long         data2,
				    long         data3)
{
  XClientMessageEvent ev;
  Display *display;

  ev.type = ClientMessage;
  ev.window = window;
  ev.message_type = icon->system_tray_opcode_atom;
  ev.format = 32;
  ev.data.l[0] = gdk_x11_get_server_time (gtk_widget_get_window(GTK_WIDGET (icon)));
  ev.data.l[1] = message;
  ev.data.l[2] = data1;
  ev.data.l[3] = data2;
  ev.data.l[4] = data3;

#if GTK_CHECK_VERSION(2,1,0)
  display = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));
#else
  display = gdk_display;
#endif

  gdk_error_trap_push ();
  XSendEvent (display,
	      icon->manager_window, False, NoEventMask, (XEvent *)&ev);
  XSync (display, False);
  gdk_error_trap_pop_ignored ();
}

static void
egg_tray_icon_send_dock_request (EggTrayIcon *icon)
{
  egg_tray_icon_send_manager_message (icon,
				      SYSTEM_TRAY_REQUEST_DOCK,
				      icon->manager_window,
				      gtk_plug_get_id (GTK_PLUG (icon)),
				      0, 0);
}

static void
egg_tray_icon_update_manager_window (EggTrayIcon *icon,
				     gboolean     dock_if_realized)
{
  Display *xdisplay;

  if (icon->manager_window != None)
    return;

#if GTK_CHECK_VERSION(2,1,0)
  xdisplay = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));
#else
  xdisplay = gdk_display;
#endif

  XGrabServer (xdisplay);

  icon->manager_window = XGetSelectionOwner (xdisplay,
					     icon->selection_atom);

  if (icon->manager_window != None)
    XSelectInput (xdisplay,
		  icon->manager_window, StructureNotifyMask|PropertyChangeMask);

  XUngrabServer (xdisplay);
  XFlush (xdisplay);

  if (icon->manager_window != None)
    {
      GdkWindow *gdkwin;

#if GTK_CHECK_VERSION(2,1,0)
      gdkwin = gdk_window_lookup_for_display (gtk_widget_get_display (GTK_WIDGET (icon)),
					      icon->manager_window);
#else
      gdkwin = gdk_window_lookup (icon->manager_window);
#endif

      gdk_window_add_filter (gdkwin, egg_tray_icon_manager_filter, icon);

      if (dock_if_realized && GTK_WIDGET_REALIZED (GTK_WIDGET (icon)))
	egg_tray_icon_send_dock_request (icon);

      egg_tray_icon_get_orientation_property (icon);
    }
}

static void
egg_tray_icon_manager_window_destroyed (EggTrayIcon *icon)
{
  GdkWindow *gdkwin;

  g_return_if_fail (icon->manager_window != None);

#if GTK_CHECK_VERSION(2,1,0)
  gdkwin = gdk_window_lookup_for_display (gtk_widget_get_display (GTK_WIDGET (icon)),
					  icon->manager_window);
#else
  gdkwin = gdk_window_lookup (icon->manager_window);
#endif

  gdk_window_remove_filter (gdkwin, egg_tray_icon_manager_filter, icon);

  icon->manager_window = None;

  egg_tray_icon_update_manager_window (icon, TRUE);
}

#if !GTK_CHECK_VERSION(2,91,0)
static gboolean
transparent_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	gdk_window_clear_area (gtk_widget_get_window(widget), event->area.x, event->area.y,
	                      event->area.width, event->area.height);
	return FALSE;
}
#else
static gboolean
transparent_expose_event (GtkWidget *widget, cairo_t *event, gpointer user_data)
{
	guint width, height;
	GdkRGBA color;
	width = gtk_widget_get_allocated_width (widget);
	height = gtk_widget_get_allocated_height (widget);
	cairo_arc (event, width / 2.0, height / 2.0,
	           MIN (width, height) / 2.0, 0, 2 * G_PI);
	gtk_style_context_get_color (gtk_widget_get_style_context (widget),
	                             0, &color);
	gdk_cairo_set_source_rgba (event, &color);
	cairo_fill (event);
	return FALSE;
}
#endif

static void
make_transparent_again (GtkWidget *widget, GtkStyle *previous_style,
                       gpointer user_data)
{
#if !GTK_CHECK_VERSION(2,91,0)
	gdk_window_set_back_pixmap(gtk_widget_get_window(widget), NULL, TRUE);
#else
	gdk_window_set_background_pattern(gtk_widget_get_window(widget), NULL);
#endif
}

static void
make_transparent (GtkWidget *widget, gpointer user_data)
{
#if GTK_CHECK_VERSION(2,19,3)
	if (GTK_WIDGET_NO_WINDOW (widget) || gtk_widget_get_app_paintable (widget))
#else
	if (GTK_WIDGET_NO_WINDOW (widget) || GTK_WIDGET_APP_PAINTABLE (widget))
#endif
		return;

	gtk_widget_set_app_paintable (widget, TRUE);
	gtk_widget_set_double_buffered (widget, FALSE);
#if !GTK_CHECK_VERSION(2,91,0)
	gdk_window_set_back_pixmap (gtk_widget_get_window(widget), NULL, TRUE);
	g_signal_connect (widget, "expose_event",
	                 G_CALLBACK (transparent_expose_event), NULL);
#else
	gdk_window_set_background_pattern (gtk_widget_get_window(widget), NULL);
	g_signal_connect (widget, "draw",
	                 G_CALLBACK (transparent_expose_event), NULL);
#endif
	g_signal_connect_after (widget, "style_set",
	                       G_CALLBACK (make_transparent_again), NULL);
}

static void
egg_tray_icon_realize (GtkWidget *widget)
{
  EggTrayIcon *icon = EGG_TRAY_ICON (widget);
  gint screen = -1;
  Display *xdisplay;
  char buffer[256];
  GdkWindow *root_window;

  if (GTK_WIDGET_CLASS (parent_class)->realize)
    GTK_WIDGET_CLASS (parent_class)->realize (widget);

  make_transparent (widget, NULL);

#if GTK_CHECK_VERSION(2,1,0)
  GdkScreen *Screen = gtk_widget_get_screen (widget);
  if (Screen) screen = gdk_screen_get_number (Screen);
  xdisplay = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (widget));
#else
  screen = XScreenNumberOfScreen (DefaultScreenOfDisplay (gdk_display));
  xdisplay = gdk_display;
#endif

  /* Now see if there's a manager window around */
  g_snprintf (buffer, sizeof (buffer),
	      "_NET_SYSTEM_TRAY_S%d",
	      screen);

  icon->selection_atom = XInternAtom (xdisplay, buffer, False);

  icon->manager_atom = XInternAtom (xdisplay, "MANAGER", False);

  icon->system_tray_opcode_atom = XInternAtom (xdisplay,
						   "_NET_SYSTEM_TRAY_OPCODE",
						   False);

  icon->orientation_atom = XInternAtom (xdisplay,
					"_NET_SYSTEM_TRAY_ORIENTATION",
					False);

  egg_tray_icon_update_manager_window (icon, FALSE);
  egg_tray_icon_send_dock_request (icon);

#if GTK_CHECK_VERSION(2,1,0)
  if (Screen)
    root_window = gdk_screen_get_root_window (Screen);
  else
    return;
#else
  root_window = gdk_window_lookup (gdk_x11_get_default_root_xwindow ());
#endif

  /* Add a root window filter so that we get changes on MANAGER */
  gdk_window_add_filter (root_window,
			 egg_tray_icon_manager_filter, icon);
}

static void
egg_tray_icon_add (GtkContainer *container, GtkWidget *widget)
{
	g_signal_connect (widget, "realize",
	                 G_CALLBACK (make_transparent), NULL);
	GTK_CONTAINER_CLASS (parent_class)->add (container, widget);
}

#if GTK_CHECK_VERSION(2,1,0)
EggTrayIcon *
egg_tray_icon_new_for_screen (GdkScreen *screen, const char *name)
{
  g_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);

  return (EggTrayIcon*)g_object_new (EGG_TYPE_TRAY_ICON, "screen", screen, "title", name, NULL);
}
#endif

EggTrayIcon*
egg_tray_icon_new (const gchar *name)
{
  return (EggTrayIcon*)g_object_new (EGG_TYPE_TRAY_ICON, "title", name, NULL);
}

guint
egg_tray_icon_send_message (EggTrayIcon *icon,
			    gint         timeout,
			    const gchar *message,
			    gint         len)
{
  guint stamp;

  g_return_val_if_fail (EGG_IS_TRAY_ICON (icon), 0);
  g_return_val_if_fail (timeout >= 0, 0);
  g_return_val_if_fail (message != NULL, 0);

  if (icon->manager_window == None)
    return 0;

  if (len < 0)
    len = strlen (message);

  stamp = icon->stamp++;

  /* Get ready to send the message */
  egg_tray_icon_send_manager_message (icon, SYSTEM_TRAY_BEGIN_MESSAGE,
				      (Window)gtk_plug_get_id (GTK_PLUG (icon)),
				      timeout, len, stamp);

  /* Now to send the actual message */
  gdk_error_trap_push ();
  while (len > 0)
    {
      XClientMessageEvent ev;
      Display *xdisplay;

#if GTK_CHECK_VERSION(2,1,0)
      xdisplay = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));
#else
      xdisplay = gdk_display;
#endif

      ev.type = ClientMessage;
      ev.window = (Window)gtk_plug_get_id (GTK_PLUG (icon));
      ev.format = 8;
      ev.message_type = XInternAtom (xdisplay,
				     "_NET_SYSTEM_TRAY_MESSAGE_DATA", False);
      if (len > 20)
	{
	  memcpy (&ev.data, message, 20);
	  len -= 20;
	  message += 20;
	}
      else
	{
	  memcpy (&ev.data, message, len);
	  len = 0;
	}

      XSendEvent (xdisplay,
		  icon->manager_window, False, StructureNotifyMask, (XEvent *)&ev);
      XSync (xdisplay, False);
    }
  gdk_error_trap_pop_ignored ();

  return stamp;
}

void
egg_tray_icon_cancel_message (EggTrayIcon *icon,
			      guint        id)
{
  g_return_if_fail (EGG_IS_TRAY_ICON (icon));
  g_return_if_fail (id > 0);

  egg_tray_icon_send_manager_message (icon, SYSTEM_TRAY_CANCEL_MESSAGE,
				      (Window)gtk_plug_get_id (GTK_PLUG (icon)),
				      id, 0, 0);
}

GtkOrientation
egg_tray_icon_get_orientation (EggTrayIcon *icon)
{
  g_return_val_if_fail (EGG_IS_TRAY_ICON (icon), GTK_ORIENTATION_HORIZONTAL);

  return icon->orientation;
}
