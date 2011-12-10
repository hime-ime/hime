/******************************************************************

         Copyright 1994, 1995 by Sun Microsystems, Inc.
         Copyright 1993, 1994 by Hewlett-Packard Company

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and Hewlett-Packard not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and Hewlett-Packard make no representations about
the suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

SUN MICROSYSTEMS INC. AND HEWLETT-PACKARD COMPANY DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
SUN MICROSYSTEMS, INC. AND HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  Author: Hidetoshi Tajima(tajima@Eng.Sun.COM) Sun Microsystems, Inc.

******************************************************************/
#if UNIX
typedef struct {
    XRectangle	area;		/* area */
    XRectangle	area_needed;	/* area needed */
    Colormap	cmap;		/* colormap */
    CARD32	foreground;	/* foreground */
    CARD32	background;	/* background */
    Pixmap	bg_pixmap;	/* background pixmap */
    char	*base_font;	/* base font of fontset */
    CARD32	line_space;	/* line spacing */
    Cursor	cursor;		/* cursor */
} PreeditAttributes;

typedef struct {
    XRectangle	area;		/* area */
    XRectangle	area_needed;	/* area needed */
    Colormap	cmap;		/* colormap */
    CARD32	foreground;	/* foreground */
    CARD32	background;	/* background */
    Pixmap	bg_pixmap;	/* background pixmap */
    char	*base_font;	/* base font of fontset */
    CARD32	line_space;	/* line spacing */
    Cursor	cursor;		/* cursor */
} StatusAttributes;
#endif

typedef struct {
    Window	client_win;	/* client window */
    INT32	input_style;	/* input style */
    HIME_STATE_E im_state;
    gboolean    b_half_full_char;
    gboolean    fixed_pos;
    gboolean    b_hime_protocol; // TRUE : hime    FALSE: XIM
    gboolean    b_raise_window;
    gboolean    use_preedit;
    gboolean    tsin_pho_mode;
    short       fixed_x, fixed_y;
    short       in_method;
    XPoint	spot_location;	/* spot location, relative to client window */
#if USE_XIM
    gboolean xim_preedit_started;
#endif
} ClientState;


typedef struct _IC {
#if USE_XIM
    CARD16	id;		/* ic id */
#endif
    Window	focus_win;	/* focus window */
#if USE_XIM
    char	*resource_name;	/* resource name */
    char	*resource_class; /* resource class */
    PreeditAttributes pre_attr; /* preedit attributes */
    StatusAttributes sts_attr; /* status attributes */
#endif
    ClientState cs;
    struct _IC	*next;
} IC;

typedef struct {
//  char *server_locale;
  char xim_server_name[32];
  Window xim_xwin;
#if USE_XIM
  XIMS xims;
#endif
} DUAL_XIM_ENTRY;

Window get_ic_win(IC *rec);
