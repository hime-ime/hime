/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include "hime.h"
#include "pho.h"
#include "win-sym.h"
#include "gst.h"

/* "destroy_window = FALSE" should be ok with both GTK+ 2.x and 3.x
 * gcin use TRUE for GTK+ 3.x, but caleb- always patch it to FALSE
 */
#if 0
int destroy_window = TRUE;
#else
int destroy_window = FALSE;
#endif

GtkWidget *gwin0 = NULL;
extern GtkWidget *gwin1;
extern Display *dpy;
static GtkWidget *top_bin;
int current_hime_inner_frame;

static GtkWidget *hbox_edit;
static PangoAttrList* attr_list, *attr_list_blank;
extern gboolean test_mode;

void compact_win0();
void move_win0(int x, int y);
void get_win0_geom();

static struct {
  GtkWidget *vbox;
  GtkWidget *label;
//  GtkWidget *line;
  int x;
} chars[MAX_PH_BF_EXT];


static GtkWidget *button_pho;
static GtkWidget *label_pho;
extern char text_pho[];
extern int text_pho_N;
static GtkWidget *button_eng_ph;
//static int max_yl;

static void create_win0_gui();

static void recreate_win0()
{
  bzero(chars, sizeof(chars));
  label_pho = NULL;

  create_win0_gui();
}

#if WIN32
static int timeout_handle;
gboolean timeout_minimize_win0(gpointer data)
{
  if (!gwin0)
    return FALSE;
  gtk_window_resize(GTK_WINDOW(gwin0), 1, 1);
//  gtk_window_present(GTK_WINDOW(gwin0));
  timeout_handle = 0;
  return FALSE;
}
#endif


#if USE_TSIN
void change_win0_style()
{
  if (!top_bin || current_hime_inner_frame == hime_inner_frame)
    return;

  gtk_widget_destroy(top_bin);
  top_bin = NULL;

  current_hime_inner_frame = hime_inner_frame;
  recreate_win0();
}
#endif


void set_label_font_size();

/* there is a bug in gtk, if the widget is created and hasn't been processed by
   gtk_main(), the coodinate of the widget is sometimes invalid.
   We use pre-create to overcome this bug.
*/

void drawcursor();
void open_select_pho();
void create_phrase_save_menu(GdkEventButton * event);

static void mouse_char_callback( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
  tss.c_idx = GPOINTER_TO_INT(data);
  drawcursor();

  switch (event->button) {
    case 1:
    case 2:
      open_select_pho();
      break;
    case 3:
    {
      create_phrase_save_menu(event);
      break;
    }
  }
}

static void create_char(int index)
{
  int i;

  if (!hbox_edit)
    return;

  GdkColor fg;
  gdk_color_parse(hime_win_color_fg, &fg);
  GdkColor color_bg;
  gdk_color_parse(tsin_phrase_line_color, &color_bg);


  i = index;
  {
    if (chars[i].vbox)
      return;

    GtkWidget *event_box = gtk_event_box_new();
    gtk_event_box_set_visible_window (GTK_EVENT_BOX(event_box), FALSE);
    chars[i].vbox = event_box;
    g_signal_connect (G_OBJECT (event_box), "button-press-event",  G_CALLBACK (mouse_char_callback), GINT_TO_POINTER(index));

    gtk_box_pack_start (GTK_BOX (hbox_edit), event_box, FALSE, FALSE, 0);
    GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(event_box), vbox);

    GtkWidget *label = gtk_label_new(NULL);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

    set_label_font_size(label, hime_font_size);
    chars[i].label = label;

    if (hime_win_color_use) {
#if !GTK_CHECK_VERSION(2,91,6)
      gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &fg);
#else
      GdkRGBA rgbfg;
      gdk_rgba_parse(&rgbfg, gdk_color_to_string(&fg));
      gtk_widget_override_color(label, GTK_STATE_FLAG_NORMAL, &rgbfg);
#endif
    }

    gtk_widget_show_all(event_box);
  }
}

extern gboolean b_use_full_space;

void set_label_space();
void set_label_space(GtkWidget *label);

void disp_char(int index, char *ch)
{
  if (hime_edit_display_ap_only())
    return;
  if (!top_bin)
    return;

//  dbg("disp_char %d %s\n", index, ch);
  create_char(index);
  GtkWidget *label = chars[index].label;

  if (label) {
    if (ch[0]==' ' && ch[1]==' ')
      set_label_space(label);
    else {
      gtk_label_set_text(GTK_LABEL(label), ch);
    }
  }

  get_win0_geom();
  if (win_x + win_xl >= dpy_xl)
    move_win0(dpy_xl - win_xl, win_y);

  gtk_widget_show_all(chars[index].vbox);
}

void hide_char(int index)
{
#if WIN32
  if (test_mode)
    return;
#endif
  if (!chars[index].vbox)
    return;
  gtk_label_set_text(GTK_LABEL(chars[index].label), "");
  gtk_widget_hide_all(chars[index].vbox);
}


void clear_chars_all()
{
  int i;
#if WIN32
  if (test_mode)
    return;
#endif
  for(i=0; i < MAX_PH_BF_EXT; i++) {
    hide_char(i);
  }

  compact_win0();
}

void set_cursor_tsin(int index)
{
  GtkWidget *label = chars[index].label;

  if (!label)
    return;

  if (hime_edit_display_ap_only())
    return;

  gtk_label_set_attributes(GTK_LABEL(label), attr_list);
}

void clr_tsin_cursor(int index)
{
  GtkWidget *label = chars[index].label;

  if (!label)
    return;
#if WIN32
  if (test_mode)
    return;
#endif
  gtk_label_set_attributes(GTK_LABEL(label), attr_list_blank);
}

void disp_pho_sub(GtkWidget *label, int index, char *pho);
void hide_win0();

void disp_tsin_pho(int index, char *pho)
{
#if WIN32
  if (test_mode)
    return;
#endif
  if (hime_display_on_the_spot_key()) {
    if (gwin0 && GTK_WIDGET_VISIBLE(gwin0))
      hide_win0();
    return;
  }

  if (button_pho && !GTK_WIDGET_VISIBLE(button_pho))
    gtk_widget_show(button_pho);

  text_pho_N = pin_juyin?6:3;
  disp_pho_sub(label_pho, index, pho);
}

void disp_tsin_pho(int index, char *pho);
void clr_in_area_pho_tsin()
{
  int i;
#if WIN32
  if (test_mode)
    return;
#endif
  for(i=0; i < text_pho_N; i++)
   disp_tsin_pho(i, " ");
}

int get_widget_xy(GtkWidget *win, GtkWidget *widget, int *rx, int *ry)
{
  if (!win && !widget)
    p_err("get_widget_xy err");

//  gdk_flush();

  GtkRequisition sz;
  gtk_widget_get_preferred_size(widget, NULL, &sz);
  int wx, wy;

  wx=wy=0;

  gtk_widget_translate_coordinates(widget, win,
         0, sz.height, &wx, &wy);

  gtk_widget_translate_coordinates(widget, win,
         0, sz.height, &wx, &wy);

//  dbg("%d wx:%d\n", index,  wx);

  int win_x, win_y;

  gtk_window_get_position(GTK_WINDOW(win), &win_x, &win_y);
  int win_xl, win_yl;
  get_win_size(win, &win_xl, &win_yl);

  if (wx > win_xl)
    wx = win_xl;

  *rx = win_x + wx;
  *ry = win_y + wy;
  return wx;
}

void getRootXY(Window win, int wx, int wy, int *tx, int *ty);
void disp_selections(int x, int y);
void disp_tsin_select(int index)
{
  int x,y;
#if WIN32
  if (test_mode)
    return;
#endif

  if (index < 0)
    return;

//  dbg("hime_edit_display_ap_only() %d\n", hime_edit_display_ap_only());

  if (hime_edit_display_ap_only()) {
    getRootXY(current_CS->client_win, current_CS->spot_location.x, current_CS->spot_location.y, &x, &y);
  } else {
#if 1
    int i;
    // bug in GTK, widget position is wrong, repeat util find one
    for(i=index;i>=0; i--) {
      gtk_widget_show_now(chars[i].label);
      gtk_widget_show(chars[i].vbox);
      gtk_main_iteration_do(FALSE);

      int tx = get_widget_xy(gwin0, chars[i].vbox, &x, &y);

      if (tx>=0)
        break;
    }
#else
	get_widget_xy(gwin0, chars[index].vbox, &x, &y);
#endif
	get_win0_geom();
  }
  disp_selections(x, y);
}

#define MIN_X_SIZE 32

static int best_win_x, best_win_y;

static void raw_move(int x, int y)
{
  int xl, yl;

  if (!gwin0)
    return;

  get_win_size(gwin0, &xl, &yl);

  if (x + xl > dpy_xl)
    x = dpy_xl - xl;
  if (y + yl > dpy_yl)
    y = dpy_yl - yl;

  gtk_window_move(GTK_WINDOW(gwin0), x, y);
//  dbg("gwin0:%x raw_move %d,%d\n", gwin0, x, y);
}

#if 0
void compact_win0_x()
{
#if WIN32
  if (test_mode)
    return;
#endif
  if (!gwin0)
    return;

  gtk_window_resize(GTK_WINDOW(gwin0), 1, 1);
  raw_move(best_win_x, best_win_y);
#if WIN32
  if (!timeout_handle)
	timeout_handle = g_timeout_add(50, timeout_minimize_win0, NULL);
#endif
}
#endif

void compact_win0()
{
#if WIN32
  if (test_mode)
    return;
#endif

  if (!gwin0)
    return;

//  max_yl = 0;
  gtk_window_resize(GTK_WINDOW(gwin0), 1, 1);
  raw_move(best_win_x, best_win_y);

#if WIN32
  if (!timeout_handle)
	timeout_handle = g_timeout_add(50, timeout_minimize_win0, NULL);
#endif
}

gboolean tsin_has_input();
GtkWidget *gwin_sym;

void move_win0(int x, int y)
{
//  dbg("--- gwin0:%x move_win0 %d,%d\n", gwin0, x,y);
  best_win_x = x;
  best_win_y = y;

  if (gwin0)
    gtk_window_get_size(GTK_WINDOW(gwin0), &win_xl, &win_yl);

  if (x + win_xl > dpy_xl)
    x = dpy_xl - win_xl;
  if (x < 0)
    x = 0;

  if (y + win_yl > dpy_yl)
    y = dpy_yl - win_yl;
  if (y < 0)
    y = 0;

//  dbg("move_win0 %d,%d\n",x, y);

  if (gwin0)
    gtk_window_move(GTK_WINDOW(gwin0), x, y);

//  dbg("move_win0 %d %d\n",x,y);
  win_x = x;
  win_y = y;

#if WIN32 && 0
  if (gwin1 && GTK_WIDGET_VISIBLE(gwin1)) {
    gtk_window_move(GTK_WINDOW(gwin1), x, y);
  }
#endif

  move_win_sym();
}


void disp_tsin_eng_pho(int eng_pho)
{
  static unich_t *eng_pho_strs[]={_L("英"),_L("注")};

  if (!button_eng_ph)
    return;

  gtk_button_set_label(GTK_BUTTON(button_eng_ph), _(eng_pho_strs[eng_pho]));
}

void exec_hime_setup();
void toggle_win_sym();

static void mouse_button_callback( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
//  dbg("mouse_button_callback %d\n", event->button);
  switch (event->button) {
    case 1:
      toggle_win_sym();
      break;
    case 2:
      inmd_switch_popup_handler(widget, (GdkEvent *)event);
      break;
    case 3:
      exec_hime_setup();
      break;
  }
}


void tsin_toggle_eng_ch();
void set_no_focus();


void create_win0()
{
  if (gwin0)
    return;
#if _DEBUG && 0
  dbg("create_win0\n");
#endif
  gwin0 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(gwin0), FALSE);
#if WIN32
  set_no_focus(gwin0);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (gwin0), 0);
  gtk_widget_realize (gwin0);
#if UNIX
  set_no_focus(gwin0);
#else
  win32_init_win(gwin0);
#endif
}


void create_win1();

static void create_cursor_attr()
{
  if (attr_list)
    pango_attr_list_unref(attr_list);

  GdkColor color_bg, color_fg;
  gdk_color_parse(tsin_cursor_color, &color_bg);
  gdk_color_parse("white", &color_fg);

  attr_list = pango_attr_list_new ();
  attr_list_blank = pango_attr_list_new ();

  PangoAttribute *blue_bg = pango_attr_background_new(
    color_bg.red, color_bg.green, color_bg.blue);
  blue_bg->start_index = 0;
  blue_bg->end_index = 128;
  pango_attr_list_insert (attr_list, blue_bg);

  PangoAttribute *white_fg = pango_attr_foreground_new(
    color_fg.red, color_fg.green, color_fg.blue);
  white_fg->start_index = 0;
  white_fg->end_index = 128;
  pango_attr_list_insert (attr_list, white_fg);
}

void init_tsin_selection_win();

static void set_win0_bg()
{
#if 1
  change_win_bg(gwin0);
#endif
}

void change_win1_font();

static void create_win0_gui()
{
  if (top_bin)
    return;

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_top), GTK_ORIENTATION_VERTICAL);
  gtk_container_set_border_width (GTK_CONTAINER (gwin0), 0);

  if (hime_inner_frame) {
    GtkWidget *frame;
    top_bin = frame = gtk_frame_new(NULL);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
    gtk_container_add (GTK_CONTAINER(gwin0), frame);
    gtk_container_add (GTK_CONTAINER (frame), vbox_top);
  } else {
    top_bin = vbox_top;
    gtk_container_add (GTK_CONTAINER (gwin0), vbox_top);
  }

  bzero(chars, sizeof(chars));

  GtkWidget *hbox_row1 = gtk_hbox_new (FALSE, 0);
  /* This packs the button into the gwin0 (a gtk container). */
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row1, FALSE, FALSE, 0);

  hbox_edit = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox_edit), 0);
  /* This packs the button into the gwin0 (a gtk container). */
  gtk_box_pack_start (GTK_BOX (hbox_row1), hbox_edit, FALSE, FALSE, 0);

  create_cursor_attr();

  button_pho = gtk_button_new();
  gtk_container_set_border_width (GTK_CONTAINER (button_pho), 0);
  gtk_box_pack_start (GTK_BOX (hbox_row1), button_pho, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(button_pho),"button-press-event",
                   G_CALLBACK(mouse_button_callback), NULL);
#if GTK_CHECK_VERSION(2,18,0)
   gtk_widget_set_can_focus(button_pho, FALSE);
   gtk_widget_set_can_default(button_pho, FALSE);
#else
  GTK_WIDGET_UNSET_FLAGS(button_pho,  GTK_CAN_FOCUS|GTK_CAN_DEFAULT);
#endif

  if (left_right_button_tips) {
#if GTK_CHECK_VERSION(2,12,0)
    gtk_widget_set_tooltip_text (button_pho, _(_L("左鍵符號，右鍵設定")));
#else
    GtkTooltips *button_pho_tips = gtk_tooltips_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_pho_tips), button_pho, _(_L("左鍵符號，右鍵設定")),NULL);
#endif
  }

  label_pho = gtk_label_new("");
  set_label_font_size(label_pho, hime_font_size_tsin_pho_in);
  gtk_container_add (GTK_CONTAINER (button_pho), label_pho);

  clr_in_area_pho_tsin();

  gtk_widget_show_all (gwin0);
//  gdk_flush();
  gtk_widget_hide(gwin0);

  init_tsin_selection_win();

  set_win0_bg();

//  change_win1_font();
}

static void destroy_top_bin()
{
  if (!top_bin)
    return;
  gtk_widget_destroy(top_bin);
  top_bin = NULL;
  label_pho = NULL;
  button_pho = NULL;
  button_eng_ph = NULL;
  hbox_edit = NULL;
  bzero(chars, sizeof(chars));
}

void destroy_win0()
{
  if (!gwin0)
    return;
  destroy_top_bin();
  gtk_widget_destroy(gwin0);
  gwin0 = NULL;
}

void get_win0_geom()
{
  if (!gwin0)
    return;
  gtk_window_get_position(GTK_WINDOW(gwin0), &win_x, &win_y);
  get_win_size(gwin0, &win_xl, &win_yl);
}

gboolean tsin_has_input();
extern gboolean force_show;
void raise_tsin_selection_win();

void show_win0()
{
#if WIN32
  if (test_mode)
    return;
#endif

#if _DEBUG && 1
	dbg("show_win0 pop:%d in:%d for:%d \n", hime_pop_up_win, tsin_has_input(), force_show);
#endif
  create_win0();
  create_win0_gui();

  if (hime_pop_up_win && !tsin_has_input() && !force_show) {
//    dbg("show ret\n");
    return;
  }

#if WIN32
  compact_win0();
#endif

#if UNIX && 0
  if (!GTK_WIDGET_VISIBLE(gwin0))
#endif
  {
//    dbg("gtk_widget_show %x\n", gwin0);
#if UNIX
    move_win0(win_x, win_y);
    gtk_widget_show(gwin0);
#else
	move_win0(win_x, win_y);
    gtk_widget_show(gwin0);
//    move_win0(win_x, win_y);
#endif
  }

  show_win_sym();

#if UNIX
  if (current_CS->b_raise_window)
#endif
  {
    gtk_window_present(GTK_WINDOW(gwin0));
    raise_tsin_selection_win();
  }
}

void hide_selections_win();
void hide_win0()
{
#if WIN32
  if (test_mode)
    return;
#endif
  if (!gwin0)
    return;

#if WIN32
  if (timeout_handle) {
	  g_source_remove(timeout_handle);
	  timeout_handle = 0;
  }
#endif

  gtk_widget_hide(gwin0);
  if (destroy_window)
    destroy_win0();
  else
    destroy_top_bin();

  hide_selections_win();
  hide_win_sym();
}

void bell();


#if USE_TSIN
void change_tsin_font_size()
{
  if (!top_bin)
    return;

  GdkColor fg;
  gdk_color_parse(hime_win_color_fg, &fg);

  set_label_font_size(label_pho, hime_font_size_tsin_pho_in);

  int i;
  for(i=0; i < MAX_PH_BF_EXT; i++) {
    GtkWidget *label = chars[i].label;
    if (!label)
      continue;

    set_label_font_size(label, hime_font_size);

    if (hime_win_color_use) {
#if !GTK_CHECK_VERSION(2,91,6)
      gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &fg);
#else
      GdkRGBA rgbfg;
      gdk_rgba_parse(&rgbfg, gdk_color_to_string(&fg));
      gtk_widget_override_color(label, GTK_STATE_FLAG_NORMAL, &rgbfg);
#endif
    }
  }

  compact_win0();

//  change_win1_font();

  set_win0_bg();
//  change_tsin_line_color();
}
#endif



void show_button_pho(gboolean bshow)
{
  if (!button_pho)
    return;

  if (bshow)
    gtk_widget_show(button_pho);
  else {
    gtk_widget_hide(button_pho);
    compact_win0();
  }
}

char *get_full_str();

void win_tsin_disp_half_full()
{
#if WIN32
  if (test_mode)
    return;
#endif
  if (hime_win_color_use)
   gtk_label_set_markup(GTK_LABEL(label_pho), get_full_str());
  else
    gtk_label_set_text(GTK_LABEL(label_pho), get_full_str());
  compact_win0();
}


void drawcursor();

#if USE_TSIN
void change_tsin_color()
{
  create_cursor_attr();

  drawcursor();
}
#endif
