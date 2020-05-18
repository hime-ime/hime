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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/keysym.h>

#include "hime.h"

#include "gtkimcontexthime.h"
#include "hime-im-client.h"

#define DBG 0

struct _GtkIMContextHIME {
    GtkIMContext object;

    GdkWindow *client_window;

    HIME_client_handle *hime_ch;

    // preedit
    char *pe_str;
    HIME_PREEDIT_ATTR *pe_att;
    int pe_attN;
    int pe_cursor;
    gboolean pe_started;
};

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
                              HIME_PREEDIT_ATTR *att);

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
    im_context_hime->pe_att = NULL;
    im_context_hime->pe_attN = 0;
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

    if (context_hime->pe_att) {
        free (context_hime->pe_att);
        context_hime->pe_att = NULL;
        context_hime->pe_attN = 0;
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

    GdkDisplay *display = get_default_display ();
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
        hime_im_client_set_window (context_xim->hime_ch,
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

    // nowhere to send the out-string
    if (!str) {
        return;
    }

    if (!context_hime->hime_ch) {
        *str = g_strdup ("");
        return;
    }

    if (cursor_pos) {
        *cursor_pos = context_hime->pe_cursor;
    }

    if (context_hime->pe_str) {
        *str = g_strdup (context_hime->pe_str);
    } else {
        *str = g_strdup ("");
        return;
    }

    if (attrs) {
        *attrs = pango_attr_list_new ();
        for (int i = 0; i < context_hime->pe_attN; i++) {
            add_preedit_attr (*attrs, *str, &(context_hime->pe_att[i]));
        }
    }
}

static gboolean gtk_im_context_hime_filter_keypress (GtkIMContext *context,
                                                     GdkEventKey *event) {
    GtkIMContextHIME *context_xim = GTK_IM_CONTEXT_HIME (context);

    const int BUFFER_SIZE = 256;
    gchar static_buffer[BUFFER_SIZE];
    char *buffer = static_buffer;
    gint buffer_size = sizeof (static_buffer) - 1;
    gsize num_bytes = 0;
    KeySym keysym = 0;
    //  Status status;
    gboolean result = FALSE;
#if !GTK_CHECK_VERSION(3, 0, 0)
    GdkWindow *root_window = gdk_screen_get_root_window (
        gdk_window_get_screen (event->window));
#else
    GdkWindow *root_window = NULL;
    GdkScreen *screen = gdk_window_get_screen (event->window);
    if (screen)
        root_window = gdk_screen_get_root_window (screen);
    else
        return result;
#endif

    XKeyPressedEvent xevent;
    xevent.type = (event->type == GDK_KEY_PRESS) ? KeyPress : KeyRelease;
    xevent.serial = 0; /* hope it doesn't matter */
    xevent.send_event = (unsigned char) event->send_event;
    xevent.display = GDK_WINDOW_XDISPLAY (event->window);
    xevent.window = GDK_WINDOW_XID (event->window);
    xevent.root = GDK_WINDOW_XID (root_window);
    xevent.subwindow = xevent.window;
    xevent.time = event->time;
    xevent.x = xevent.x_root = 0;
    xevent.y = xevent.y_root = 0;
    xevent.state = event->state;
    xevent.keycode = event->hardware_keycode;
    xevent.same_screen = True;
    num_bytes = XLookupString (&xevent, buffer, buffer_size, &keysym, NULL);

    char *rstr = NULL;

#if (!FREEBSD)
    int uni = gdk_keyval_to_unicode (event->keyval);
    if (uni) {
        gsize rn = 0;
        GError *err = NULL;
        char *utf8 = g_convert ((char *) &uni,
                                4, "UTF-8", "UTF-32", &rn, &num_bytes, &err);

        if (utf8) {
            strncpy (buffer, utf8, BUFFER_SIZE);
            g_free (utf8);
        }
    }
#endif

    gboolean preedit_changed = FALSE;
    gboolean context_pe_started = context_xim->pe_started;
    gboolean context_has_str = context_xim->pe_str && context_xim->pe_str[0];
    char *tstr = NULL;
    int sub_comp_len = 0;
    HIME_PREEDIT_ATTR att[HIME_PREEDIT_ATTR_MAX_N];
    int cursor_pos = 0;
    gboolean has_str = FALSE;

    if (event->type == GDK_KEY_PRESS) {
        result = hime_im_client_forward_key_press (context_xim->hime_ch,
                                                   keysym, xevent.state, &rstr);
    } else {
        result = hime_im_client_forward_key_release (context_xim->hime_ch,
                                                     keysym, xevent.state, &rstr);
    }

    preedit_changed = result;

    int attN = hime_im_client_get_preedit (context_xim->hime_ch,
                                           &tstr, att, &cursor_pos, &sub_comp_len);
    has_str = tstr && tstr[0];

    if (sub_comp_len) {
        has_str = TRUE;
        //      preedit_changed = TRUE;
    }

    if (!context_pe_started && has_str) {
        g_signal_emit_by_name (context, "preedit-start");
        context_pe_started = context_xim->pe_started = TRUE;
    }

    if (context_has_str != has_str ||
        (tstr && context_xim->pe_str && (strcmp (tstr, context_xim->pe_str) != 0))) {
        if (context_xim->pe_str) {
            free (context_xim->pe_str);
        }
        context_xim->pe_str = tstr;
        //      preedit_changed = TRUE;
    }

    size_t attsz = sizeof (HIME_PREEDIT_ATTR) * attN;
    if (context_xim->pe_attN != attN ||
        (context_xim->pe_att && (memcmp (context_xim->pe_att, att, attsz) != 0))) {
        //      printf("att changed pe_att:%x:%d %d\n", context_xim->pe_att, context_xim->pe_attN, attN);
        context_xim->pe_attN = attN;
        if (context_xim->pe_att) {
            free (context_xim->pe_att);
        }

        context_xim->pe_att = NULL;
        if (attN) {
            context_xim->pe_att = malloc (attsz);
        }

        if (context_xim->pe_att) {
            memcpy (context_xim->pe_att, att, attsz);
            //      printf("context_xim->pe_att %x\n", context_xim->pe_att);
            //      preedit_changed = TRUE;
        }
    }

    if (context_xim->pe_cursor != cursor_pos) {
#if DBG
        printf ("cursor changed %d %d\n", context_xim->pe_cursor, cursor_pos);
#endif
        context_xim->pe_cursor = cursor_pos;
        //      preedit_changed = TRUE;
    }

#if DBG
    printf ("seq:%d rstr:%s result:%x num_bytes:%d %x\n", context_xim->hime_ch->seq, rstr, result, num_bytes, (unsigned int) buffer[0]);
#endif
    if (event->type == GDK_KEY_PRESS && !rstr && !result && num_bytes &&
        buffer[0] >= 0x20 && buffer[0] != 0x7f && !(xevent.state & (Mod1Mask | ControlMask))) {
        rstr = (char *) malloc (num_bytes + 1);
        memcpy (rstr, buffer, num_bytes);
        rstr[num_bytes] = 0;
        result = TRUE;
    }

    if (preedit_changed) {
        g_signal_emit_by_name (context_xim, "preedit_changed");
    }

    if (rstr) {
        g_signal_emit_by_name (context, "commit", rstr);
        free (rstr);
    }

    if (!has_str && context_pe_started) {
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
        char *rstr = NULL;
        hime_im_client_focus_out2 (context_xim->hime_ch, &rstr);

        if (rstr) {
            g_signal_emit_by_name (context, "commit", rstr);
            clear_preedit (context_xim);
            g_signal_emit_by_name (context, "preedit_changed");
            free (rstr);
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
        g_signal_emit_by_name (context, "preedit_changed");
    }
}

/*
 * Mask of feedback bits that we render
 */
static void add_preedit_attr (PangoAttrList *attrs,
                              const gchar *str,
                              HIME_PREEDIT_ATTR *att) {
    PangoAttribute *attr = NULL;
    gint start_index = g_utf8_offset_to_pointer (str, att->ofs0) - str;
    gint end_index = g_utf8_offset_to_pointer (str, att->ofs1) - str;

    if (att->flag & HIME_PREEDIT_ATTR_FLAG_UNDERLINE) {
        attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
        attr->start_index = start_index;
        attr->end_index = end_index;
        pango_attr_list_change (attrs, attr);
    }

    if (att->flag & HIME_PREEDIT_ATTR_FLAG_REVERSE) {
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
