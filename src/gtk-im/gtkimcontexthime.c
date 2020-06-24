/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * GTK - The GIMP Toolkit
 * Copyright (C) 2000 Red Hat, Inc.
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/keysym.h>

#include "gtkimcontexthime.h"
#include "hime-im-client.h"

#define DBG 0

struct _GtkIMContextHIME {
    GtkIMContext object;

    GdkWindow *client_window;

    HIME_client_handle *hime_ch;

    // preedit
    char *pe_str;
    HIME_PREEDIT_ATTR *pe_attr;
    int pe_attrN;
    int pe_cursor;
    gboolean pe_started;
};

static const int BUFFER_SIZE = 256;

// GObject functions
static void gtk_im_context_hime_class_init (GtkIMContextHIMEClass *class);
static void gtk_im_context_hime_init (GtkIMContextHIME *im_context_hime);
static void gtk_im_context_hime_finalize (GObject *obj);

// GtkIMContext functions
static void gtk_im_context_hime_set_client_window (GtkIMContext *context,
                                                   GdkWindow *client_window);
static void gtk_im_context_hime_get_preedit_string (GtkIMContext *context,
                                                    gchar **str,
                                                    PangoAttrList **attrs,
                                                    gint *cursor_pos);
static gboolean gtk_im_context_hime_filter_keypress (GtkIMContext *context,
                                                     GdkEventKey *event);
static void gtk_im_context_hime_focus_in (GtkIMContext *context);
static void gtk_im_context_hime_focus_out (GtkIMContext *context);
static void gtk_im_context_hime_reset (GtkIMContext *context);
static void gtk_im_context_hime_set_cursor_location (GtkIMContext *context,
                                                     GdkRectangle *area);
static void gtk_im_context_hime_set_use_preedit (GtkIMContext *context,
                                                 gboolean use_preedit);

static void add_preedit_attr (PangoAttrList *attrs,
                              const gchar *str,
                              HIME_PREEDIT_ATTR *hime_attr);

GType gtk_type_im_context_hime = 0;

void gtk_im_context_hime_register_type (GTypeModule *type_module) {
    static const GTypeInfo im_context_hime_info = {
        sizeof (GtkIMContextHIMEClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) gtk_im_context_hime_class_init,
        NULL, /* class_finalize */
        NULL, /* class_data */
        sizeof (GtkIMContextHIME),
        0,
        (GInstanceInitFunc) gtk_im_context_hime_init,
    };

    gtk_type_im_context_hime =
        g_type_module_register_type (type_module,
                                     GTK_TYPE_IM_CONTEXT,
                                     "GtkIMContextHIME",
                                     &im_context_hime_info, 0);
}

GtkIMContext *gtk_im_context_hime_new (void) {
    GtkIMContextHIME *result = GTK_IM_CONTEXT_HIME (
        g_object_new (GTK_TYPE_IM_CONTEXT_HIME, NULL));

    return GTK_IM_CONTEXT (result);
}

/**
 * gtk_im_context_hime_shutdown:
 *
 * Destroys all the status windows that are kept by the HIME contexts.  This
 * function should only be called by the HIME module exit routine.
 **/
void gtk_im_context_hime_shutdown (void) {
}

static void gtk_im_context_hime_class_init (GtkIMContextHIMEClass *class) {
    GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (class);
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);

    im_context_class->set_client_window = gtk_im_context_hime_set_client_window;
    im_context_class->get_preedit_string = gtk_im_context_hime_get_preedit_string;
    im_context_class->filter_keypress = gtk_im_context_hime_filter_keypress;
    im_context_class->focus_in = gtk_im_context_hime_focus_in;
    im_context_class->focus_out = gtk_im_context_hime_focus_out;
    im_context_class->reset = gtk_im_context_hime_reset;
    im_context_class->set_cursor_location = gtk_im_context_hime_set_cursor_location;
    im_context_class->set_use_preedit = gtk_im_context_hime_set_use_preedit;

    gobject_class->finalize = gtk_im_context_hime_finalize;
}

static void
init_preedit (GtkIMContextHIME *im_context_hime) {
    if (!im_context_hime) {
        return;
    }

    im_context_hime->pe_str = NULL;
    im_context_hime->pe_attr = NULL;
    im_context_hime->pe_attrN = 0;
    im_context_hime->pe_cursor = 0;
    im_context_hime->pe_started = FALSE;
}

static void
gtk_im_context_hime_init (GtkIMContextHIME *im_context_hime) {
    im_context_hime->client_window = NULL;
    im_context_hime->hime_ch = NULL;
    init_preedit (im_context_hime);
}

void clear_preedit (GtkIMContextHIME *context_hime) {
    if (!context_hime) {
        return;
    }

    if (context_hime->pe_str) {
        free (context_hime->pe_str);
        context_hime->pe_str = NULL;
    }

    if (context_hime->pe_attr) {
        free (context_hime->pe_attr);
        context_hime->pe_attr = NULL;
        context_hime->pe_attrN = 0;
    }

    context_hime->pe_cursor = 0;
    context_hime->pe_started = FALSE;
}

static void gtk_im_context_hime_finalize (GObject *obj) {
    GtkIMContextHIME *context_xim = GTK_IM_CONTEXT_HIME (obj);

    clear_preedit (context_xim);

    if (context_xim->hime_ch) {
        hime_im_client_close (context_xim->hime_ch);
        context_xim->hime_ch = NULL;
    }

    context_xim->client_window = NULL;
}

static void get_hime_im_client (GtkIMContextHIME *context_xim) {

    if (!context_xim->client_window) {
        return;
    }

    GdkDisplay *display = gdk_display_get_default ();
    if (!display) {
        return;
    }

    if (!context_xim->hime_ch) {
        context_xim->hime_ch = hime_im_client_open (
            GDK_DISPLAY_XDISPLAY (display));
        if (!context_xim->hime_ch) {
            perror ("cannot open hime_ch");
        }

        init_preedit (context_xim);
    }
}

static void gtk_im_context_hime_set_client_window (GtkIMContext *context,
                                                   GdkWindow *client_window) {
    GtkIMContextHIME *context_xim = GTK_IM_CONTEXT_HIME (context);

    if (!client_window) {
        return;
    }

    context_xim->client_window = client_window;

    get_hime_im_client (context_xim);
    if (context_xim->hime_ch) {
        hime_im_client_set_client_window (context_xim->hime_ch,
                                          GDK_WINDOW_XID (client_window));
    }
}

static void gtk_im_context_hime_get_preedit_string (
    GtkIMContext *context,
    gchar **str,
    PangoAttrList **attrs,
    gint *cursor_pos) {
    GtkIMContextHIME *context_hime = GTK_IM_CONTEXT_HIME (context);

    if (context_hime->hime_ch && cursor_pos) {
        int ret = 0;
        hime_im_client_set_flags (context_hime->hime_ch,
                                  FLAG_HIME_client_handle_use_preedit, &ret);
    }

    if (str) {
        // hime client handle not present, return an empty string
        if (!context_hime->hime_ch) {
            *str = g_strdup ("");
        } else {
            // return preedit buffer if any,
            // otherwise, return an empty string
            if (context_hime->pe_str) {
                *str = g_strdup (context_hime->pe_str);
            } else {
                *str = g_strdup ("");
            }
        }
    }

    if (cursor_pos) {
        *cursor_pos = context_hime->pe_cursor;
    }

    if (attrs) {
        *attrs = pango_attr_list_new ();
        for (int i = 0; i < context_hime->pe_attrN; i++) {
            add_preedit_attr (*attrs, *str, &(context_hime->pe_attr[i]));
        }
    }
}

// returns 0 if failed
static int construct_xevent (const GdkEventKey *event,
                             XKeyPressedEvent *xevent) {

    GdkScreen *screen = gdk_window_get_screen (event->window);
    if (!screen) {
        return 0;
    }

    GdkWindow *root_window = gdk_screen_get_root_window (screen);

    xevent->type = (event->type == GDK_KEY_PRESS) ? KeyPress : KeyRelease;
    xevent->serial = 0;
    xevent->send_event = (unsigned char) event->send_event;
    xevent->display = GDK_WINDOW_XDISPLAY (event->window);
    xevent->window = GDK_WINDOW_XID (event->window);
    xevent->root = GDK_WINDOW_XID (root_window);
    xevent->subwindow = xevent->window;
    xevent->time = event->time;
    xevent->x = 0;
    xevent->y = 0;
    xevent->x_root = 0;
    xevent->y_root = 0;
    xevent->state = event->state;
    xevent->keycode = event->hardware_keycode;
    xevent->same_screen = True;

    return 1;
}

static gboolean gtk_im_context_hime_filter_keypress (GtkIMContext *context,
                                                     GdkEventKey *event) {
    GtkIMContextHIME *context_xim = GTK_IM_CONTEXT_HIME (context);

    // buffer between X and hime
    gchar static_buffer[BUFFER_SIZE];
    char *buffer = static_buffer;
    gint buffer_size = sizeof (static_buffer) - 1;

    // TRUE if the input method handled the key event.
    // No further processing should be done for this key event for Gtk.
    gboolean result = FALSE;

    // the final result of preediting to be commited
    char *result_str = NULL;

    // construct key event
    XKeyPressedEvent xevent;
    int ok = construct_xevent (event, &xevent);
    if (!ok) {
        // can't get root window, skip processing
        return result;
    }

    // XLookupString translates a key event to a KeySym and a string,
    // returns the number of characters that are stored in the buffer.
    KeySym keysym = 0;
    gsize num_bytes = XLookupString (&xevent, buffer, buffer_size, &keysym, NULL);

#if (!FREEBSD)
    // Convert from a GDK key symbol to the corresponding ISO10646 (Unicode) character.
    // returns 0 if there is no corresponding character.
    const guint32 unicode = gdk_keyval_to_unicode (event->keyval);
    if (unicode) {
        num_bytes = g_unichar_to_utf8 (unicode, buffer);
        buffer[num_bytes] = '\0';
    }
#endif

    // tell hime-im-client to process key event
    // result_str would hold the result
    gboolean context_has_str = context_xim->pe_str && context_xim->pe_str[0];
    if (event->type == GDK_KEY_PRESS) {
        result = hime_im_client_forward_key_press (context_xim->hime_ch,
                                                   keysym, event->state, &result_str);
    } else {
        result = hime_im_client_forward_key_release (context_xim->hime_ch,
                                                     keysym, event->state, &result_str);
    }
    gboolean preedit_changed = result;

    char *preedit_str = NULL;
    HIME_PREEDIT_ATTR attr[HIME_PREEDIT_ATTR_MAX_N];
    int cursor_pos = 0;
    int sub_comp_len = 0;
    int attrN = hime_im_client_get_preedit (context_xim->hime_ch,
                                            &preedit_str, attr, &cursor_pos, &sub_comp_len);
    gboolean has_preedit_str = preedit_str && preedit_str[0];
    if (sub_comp_len) {
        has_preedit_str = TRUE;
    }

    if (!context_xim->pe_started && has_preedit_str) {
        g_signal_emit_by_name (context, "preedit-start");
        context_xim->pe_started = TRUE;
    }

    // preedit_str and pe_str hold different strings
    const gboolean different_str = preedit_str &&
                                   context_xim->pe_str &&
                                   (strcmp (preedit_str, context_xim->pe_str) != 0);

    // update preedit string
    if (context_has_str != has_preedit_str || different_str) {
        if (context_xim->pe_str) {
            free (context_xim->pe_str);
        }
        context_xim->pe_str = preedit_str;
    }

    size_t attrsz = sizeof (HIME_PREEDIT_ATTR) * attrN;

    // pe_attr and attr hold different data
    const gboolean different_attr = context_xim->pe_attr &&
                                    (memcmp (context_xim->pe_attr, attr, attrsz) != 0);

    // update pe_attr
    if (context_xim->pe_attrN != attrN || different_attr) {
        context_xim->pe_attrN = attrN;

        if (context_xim->pe_attr) {
            free (context_xim->pe_attr);
        }
        context_xim->pe_attr = NULL;

        if (attrsz) {
            context_xim->pe_attr = malloc (attrsz);

            if (context_xim->pe_attr) {
                memcpy (context_xim->pe_attr, attr, attrsz);
            }
        }
    }

    // update pe_cursor
    if (context_xim->pe_cursor != cursor_pos) {
        context_xim->pe_cursor = cursor_pos;
    }

    const gboolean alt_or_control_pressed = event->state & (Mod1Mask | ControlMask);
    // GDK_KEY_PRESS event
    // hime_im_client_forward_key_press returns False
    // result_str is empty
    // buffer[0] is printable
    // not alt_or_control_pressed
    if (event->type == GDK_KEY_PRESS &&
        !result &&
        !result_str &&
        num_bytes &&
        isprint (buffer[0]) &&
        !alt_or_control_pressed) {

        // copy buffer into result_str
        result_str = (char *) malloc (num_bytes + 1);
        memcpy (result_str, buffer, num_bytes);
        result_str[num_bytes] = 0;
        result = TRUE;
    }

    if (preedit_changed) {
        g_signal_emit_by_name (context_xim, "preedit-changed");
    }

    if (result_str) {
        g_signal_emit_by_name (context, "commit", result_str);
        free (result_str);
    }

    if (!has_preedit_str && context_xim->pe_started) {
        clear_preedit (context_xim);
        context_xim->pe_started = FALSE;
        g_signal_emit_by_name (context, "preedit-end");
    }

    return result;
}

static void gtk_im_context_hime_focus_in (GtkIMContext *context) {
    GtkIMContextHIME *context_xim = GTK_IM_CONTEXT_HIME (context);

    if (context_xim->hime_ch) {
        hime_im_client_focus_in (context_xim->hime_ch);
    }
}

static void gtk_im_context_hime_focus_out (GtkIMContext *context) {
    GtkIMContextHIME *context_xim = GTK_IM_CONTEXT_HIME (context);

    if (context_xim->hime_ch) {
        char *result_str = NULL;
        hime_im_client_focus_out2 (context_xim->hime_ch, &result_str);

        if (result_str) {
            g_signal_emit_by_name (context, "commit", result_str);
            clear_preedit (context_xim);
            g_signal_emit_by_name (context, "preedit-changed");
            free (result_str);
        }
    }
}

static void gtk_im_context_hime_set_cursor_location (GtkIMContext *context,
                                                     GdkRectangle *area) {
    if (!area) {
        return;
    }

    GtkIMContextHIME *context_xim = GTK_IM_CONTEXT_HIME (context);

    if (!context_xim->hime_ch) {
        get_hime_im_client (context_xim);
    }

    if (context_xim->hime_ch) {
        hime_im_client_set_cursor_location (
            context_xim->hime_ch,
            area->x,
            area->y + area->height);
    }
}

static void gtk_im_context_hime_set_use_preedit (GtkIMContext *context,
                                                 gboolean use_preedit) {
    GtkIMContextHIME *context_hime = GTK_IM_CONTEXT_HIME (context);

    if (!context_hime->hime_ch) {
        return;
    }

    int ret = 0;

    if (use_preedit) {
        hime_im_client_set_flags (context_hime->hime_ch,
                                  FLAG_HIME_client_handle_use_preedit, &ret);
    } else {
        hime_im_client_clear_flags (context_hime->hime_ch,
                                    FLAG_HIME_client_handle_use_preedit, &ret);
    }
}

static void gtk_im_context_hime_reset (GtkIMContext *context) {
    GtkIMContextHIME *context_hime = GTK_IM_CONTEXT_HIME (context);

    if (context_hime->hime_ch) {
        hime_im_client_reset (context_hime->hime_ch);
        clear_preedit (context_hime);
        g_signal_emit_by_name (context, "preedit-changed");
    }
}

/*
 * Mask of feedback bits that we render
 */
static void add_preedit_attr (PangoAttrList *attrs,
                              const gchar *str,
                              HIME_PREEDIT_ATTR *hime_attr) {
    PangoAttribute *attr = NULL;
    gint start_index = g_utf8_offset_to_pointer (str, hime_attr->ofs0) - str;
    gint end_index = g_utf8_offset_to_pointer (str, hime_attr->ofs1) - str;

    if (hime_attr->flag & HIME_PREEDIT_ATTR_FLAG_UNDERLINE) {
        attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
        attr->start_index = start_index;
        attr->end_index = end_index;
        pango_attr_list_change (attrs, attr);
    }

    if (hime_attr->flag & HIME_PREEDIT_ATTR_FLAG_REVERSE) {
        const guint16 rgb_min = 0x0000;
        const guint16 rgb_max = 0xffff;

        // set foreground = white
        attr = pango_attr_foreground_new (rgb_max, rgb_max, rgb_max);
        attr->start_index = start_index;
        attr->end_index = end_index;
        pango_attr_list_change (attrs, attr);

        // set background = black
        attr = pango_attr_background_new (rgb_min, rgb_min, rgb_min);
        attr->start_index = start_index;
        attr->end_index = end_index;
        pango_attr_list_change (attrs, attr);
    }
}
