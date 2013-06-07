/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation version 2.1
 * of the License.
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
#include "gtab.h"
#include "win-sym.h"
#include "gst.h"

static int current_hime_inner_frame;
static int current_gtab_in_row1;
static int current_gtab_vertical_select;
extern int destroy_window;

GtkWidget *gwin_gtab;
static GtkWidget *top_bin;
static GtkWidget *label_full, *label_gtab_sele, *label_gtab_pre_sel;
static GtkWidget *label_gtab = NULL;
static GtkWidget *label_input_method_name;
static GtkWidget *label_key_codes;
char str_key_codes[128];
gboolean better_key_codes;
static GtkWidget *box_gtab_im_name;
static GtkWidget *hbox_row2;
static GtkWidget *label_page;
static GtkWidget *label_edit;
static GdkColor better_color;
gboolean last_cursor_off;

void set_label_space(GtkWidget *label);
gboolean win_size_exceed(GtkWidget *win), gtab_phrase_on();
void move_win_gtab(int x, int y);
int win_gtab_max_key_press;

void move_gtab_pho_query_win();

static void adj_gtab_win_pos()
{
  if (!gwin_gtab)
    return;
  if (win_size_exceed(gwin_gtab))
    move_win_gtab(current_in_win_x, current_in_win_y);
}

void win_gtab_disp_half_full();

void disp_gtab(char *str)
{
  if (!label_gtab)
     return;
  if (str && (str[0]!='\0')) {
    gtk_widget_show(label_gtab);
    gtk_label_set_text(GTK_LABEL(label_gtab), str);
  } else {
    if (hime_status_tray || (! gtab_hide_row2))
      gtk_widget_hide(label_gtab);
    else
      win_gtab_disp_half_full();
  }

  adj_gtab_win_pos();
}

#if !GTK_CHECK_VERSION(2,91,6)
void set_gtab_input_color(GdkColor *color)
{
  if (label_gtab) 
    gtk_widget_modify_fg(label_gtab, GTK_STATE_NORMAL, color);
}
#else
void set_gtab_input_color(GdkRGBA *rgbfg)
{
  if (label_gtab)
    gtk_widget_override_color(label_gtab, GTK_STATE_FLAG_NORMAL, rgbfg);
}
#endif

void set_gtab_input_error_color()
{
#if !GTK_CHECK_VERSION(2,91,6)
  GdkColor red;
  gdk_color_parse("red", &red);
#else
  GdkRGBA red;
  gdk_rgba_parse(&red, "red");
#endif
  set_gtab_input_color(&red);
}

void clear_gtab_input_error_color()
{
  set_gtab_input_color(NULL);
}


static gboolean need_label_edit();

void gtab_disp_empty(char *tt, int N)
{
  int i;

  if (!need_label_edit())
    return;

  for (i=0;i < N; i++)
//    strcat(tt, "﹍");
    strcat(tt, "　"); /* Full width space */
}

void clear_gtab_in_area()
{
  if (!cur_inmd)
    return;
  char tt[64];
  tt[0]=0;
  gtab_disp_empty(tt, win_gtab_max_key_press);
  disp_gtab(tt);
}


static void set_disp_im_name();

void change_win_bg(GtkWidget *win)
{
  if (!hime_win_color_use) {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_bg(win, GTK_STATE_NORMAL, NULL);
#else
    gtk_widget_override_background_color(win, GTK_STATE_FLAG_NORMAL, NULL);
#endif
    return;
  }

  GdkColor col;
  gdk_color_parse(hime_win_color_bg, &col);
#if !GTK_CHECK_VERSION(2,91,6)
  gtk_widget_modify_bg(win, GTK_STATE_NORMAL, &col);
#else
  GdkRGBA rgbbg;
  gdk_rgba_parse(&rgbbg, gdk_color_to_string(&col));
  gtk_widget_override_background_color(win, GTK_STATE_FLAG_NORMAL, &rgbbg);
#endif
}

void change_win_fg_bg(GtkWidget *win, GtkWidget *label)
{
  if (win)
    change_win_bg(win);
// TODO: Simplify repeating codes below.
  if (!hime_win_color_use) {
#if !GTK_CHECK_VERSION(2,91,6)
    if (label)
      gtk_widget_modify_fg(label, GTK_STATE_NORMAL, NULL);
    if (label_edit)
      gtk_widget_modify_fg(label_edit, GTK_STATE_NORMAL, NULL);
    if (GTK_IS_WIDGET(label_gtab_pre_sel))
      gtk_widget_modify_fg(label_gtab_pre_sel, GTK_STATE_NORMAL, NULL);
#else
    if (label)
      gtk_widget_override_color(label, GTK_STATE_FLAG_NORMAL, NULL);
    if (label_edit)
      gtk_widget_override_color(label_edit, GTK_STATE_FLAG_NORMAL, NULL);
    if (GTK_IS_WIDGET(label_gtab_pre_sel))
      gtk_widget_override_color(label_gtab_pre_sel, GTK_STATE_FLAG_NORMAL, NULL);
#endif
    return;
  }

  GdkColor col;
  gdk_color_parse(hime_win_color_fg, &col);
#if !GTK_CHECK_VERSION(2,91,6)
  if (label)
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &col);
  if (label_edit)
    gtk_widget_modify_fg(label_edit, GTK_STATE_NORMAL, &col);
  if (label_gtab_pre_sel)
    gtk_widget_modify_fg(label_gtab_pre_sel, GTK_STATE_NORMAL, &col);
#else
  GdkRGBA rgbfg;
  gdk_rgba_parse(&rgbfg, gdk_color_to_string(&col));
  if (label)
    gtk_widget_override_color(label, GTK_STATE_FLAG_NORMAL, &rgbfg);
  if (label_edit)
    gtk_widget_override_color(label_edit, GTK_STATE_FLAG_NORMAL, &rgbfg);
  if (label_gtab_pre_sel)
    gtk_widget_override_color(label_gtab_pre_sel, GTK_STATE_FLAG_NORMAL, &rgbfg);
#endif
}



void change_gtab_font_size()
{
  if (!label_gtab_sele)
    return;

  set_label_font_size(label_gtab_sele, hime_font_size);
  set_label_font_size(label_gtab_pre_sel, hime_font_size_tsin_presel);

  set_label_font_size(label_edit, hime_font_size);

  if (GTK_IS_WIDGET (label_gtab)) set_label_font_size(label_gtab, hime_font_size_gtab_in);

  set_disp_im_name();

  change_win_fg_bg(gwin_gtab, label_gtab_sele);
}

void show_win_gtab();

void disp_gtab_sel(char *s)
{
//  dbg("disp_gtab_sel '%s' %x\n", s, label_gtab_sele);

  if (!label_gtab_sele) {
    if (s && *s)
      show_win_gtab();
    else
      return;
  }

  if (s && (!s[0]) && (hime_edit_display == HIME_EDIT_DISPLAY_ON_THE_SPOT) && gtab_hide_row2 && (hime_on_the_spot_key || (! gtab_in_row1)))
    gtk_widget_hide(gwin_gtab);
  else {
    if (gwin_gtab && !GTK_WIDGET_VISIBLE(gwin_gtab))
      show_win_gtab();
    gtk_widget_show(label_gtab_sele);
  }

//  dbg("disp_gtab_sel '%s'\n", s);
  gtk_label_set_markup(GTK_LABEL(label_gtab_sele), s);
  adj_gtab_win_pos();
}

void set_key_codes_label(char *s, int better)
{
//  dbg("set_key_codes_label %s %x\n", s, label_key_codes);
  if (!label_key_codes)
    return;
  if (s && strlen(s)) {
    if (hbox_row2 && (!gtab_hide_row2 || ggg.wild_mode || (str_key_codes[0]))) {
      gtk_widget_show(hbox_row2);
    }
  } else {
    if (gtab_hide_row2 && hbox_row2) {
      gtk_widget_hide(hbox_row2);
    }
  }

  if (better) {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_fg(label_key_codes, GTK_STATE_NORMAL, &better_color);
#else
    GdkRGBA rgbfg;
    gdk_rgba_parse(&rgbfg, gdk_color_to_string(&better_color));
    gtk_widget_override_color(label_key_codes, GTK_STATE_FLAG_NORMAL, &rgbfg);
#endif
  } else
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_fg(label_key_codes, GTK_STATE_NORMAL, NULL);
#else
    gtk_widget_override_color(label_key_codes, GTK_STATE_FLAG_NORMAL, NULL);
#endif

  gtk_label_set_text(GTK_LABEL(label_key_codes), s);
  if (s && s[0])
    gtk_widget_show(label_key_codes);

  better_key_codes = better;
  if (s && s != str_key_codes)
    strcpy(str_key_codes, s);
  else
    str_key_codes[0]=0;
}


void set_page_label(char *s)
{
  if (!label_page)
    return;
  gtk_label_set_text(GTK_LABEL(label_page), s);
  gtk_widget_show(label_page);
}

void move_win_gtab(int x, int y)
{
  if (!gwin_gtab)
    return;
//  dbg("move_win_gtab %d %d\n", x, y);
  get_win_size(gwin_gtab, &win_xl, &win_yl);

  if (x + win_xl > dpy_xl)
    x = dpy_xl - win_xl;
  if (x < 0)
    x = 0;

  if (y + win_yl > dpy_yl)
    y = dpy_yl - win_yl;
  if (y < 0)
    y = 0;

  gtk_window_move(GTK_WINDOW(gwin_gtab), x, y);
  win_x = x;  win_y = y;

  move_win_sym();
  if (poo.same_pho_query_state != SAME_PHO_QUERY_none)
    move_gtab_pho_query_win();
}

void set_gtab_input_method_name(char *s)
{
//  dbg("set_gtab_input_method_name '%s'\n", s);
  if (!gtab_disp_im_name)
    return;
  if (!label_input_method_name)
    return;
//  dbg("set_gtab_input_method_name b '%s'\n", s);
  gtk_widget_show(label_input_method_name);
  gtk_label_set_text(GTK_LABEL(label_input_method_name), s);
}

gboolean use_tsin_sel_win();
void init_tsin_selection_win();

void create_win_gtab()
{
  if (gwin_gtab)
    return;

  gwin_gtab = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(gwin_gtab), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (gwin_gtab), 0);
  gtk_widget_realize (gwin_gtab);

  set_no_focus(gwin_gtab);

  if (use_tsin_sel_win())
    init_tsin_selection_win();
}

void exec_hime_setup();

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


void toggle_half_full_char();

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);


void show_hide_label_edit()
{
  if (!label_edit)
    return;

  if (hime_edit_display_ap_only() || !gtab_phrase_on()) {
    gtk_widget_hide(label_edit);
  } else
    gtk_widget_show(label_edit);
}


void disp_label_edit(char *str)
{
  if (!label_edit)
    return;

  show_hide_label_edit();

  gtk_label_set_markup(GTK_LABEL(label_edit), str);
}

static gboolean need_label_edit()
{
  return gtab_phrase_on() && !hime_edit_display_ap_only();
}

static int current_gtab_phrase_pre_select, current_hime_on_the_spot_key, current_gtab_disp_im_name;
gboolean gtab_vertical_select_on();

static void destroy_if_necessary()
{
  gboolean new_need_label_edit = need_label_edit();
  gboolean new_last_cursor_off = gtab_in_row1 && new_need_label_edit;

//  dbg("zzz %d %d\n", gtab_in_row1, new_need_label_edit);

  if (!top_bin ||
      (current_hime_inner_frame == hime_inner_frame &&
      current_gtab_in_row1 == gtab_in_row1 &&
          new_last_cursor_off == last_cursor_off &&
      current_gtab_vertical_select == gtab_vertical_select_on() &&
      current_gtab_phrase_pre_select == gtab_phrase_pre_select &&
      current_hime_on_the_spot_key == hime_on_the_spot_key &&
      current_gtab_disp_im_name == gtab_disp_im_name &&
      ((new_need_label_edit && label_edit) || (!new_need_label_edit && !label_edit)))
      )
    return;
#if 0
  dbg("hime_inner_frame %d,%d,  gtab_in_row1 %d,%d  cursor_off% d,%d   vert:%d,%d  edit:%d,%d\n",
    current_hime_inner_frame,hime_inner_frame, current_gtab_in_row1,gtab_in_row1,
    new_last_cursor_off, last_cursor_off, current_gtab_vertical_select, gtab_vertical_select_on(),
    new_need_label_edit, label_edit!=0);
#endif
  current_gtab_phrase_pre_select = gtab_phrase_pre_select;
  current_hime_on_the_spot_key = hime_on_the_spot_key;
  current_gtab_disp_im_name = gtab_disp_im_name;

  gtk_widget_destroy(top_bin);
  top_bin = NULL;
  label_edit = NULL;
  hbox_row2 = NULL;
}

void mod_bg_all(GtkWidget *lab, GdkColor *col);

void create_win_gtab_gui_simple()
{
//  dbg("create_win_gtab_gui ..... %d, %d\n", current_CS->use_preedit, hime_edit_display);

  destroy_if_necessary();

  if (top_bin)
    return;

//  dbg("create_win_gtab_gui_simple\n");

  last_cursor_off = FALSE;

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_top), GTK_ORIENTATION_VERTICAL);

  GtkWidget *event_box_gtab;
  if (gtab_in_area_button) {
    event_box_gtab = gtk_button_new();
#if 0
    GtkStyle *style = gtk_widget_get_style(event_box_gtab);
    style->xthickness =0;
    style->ythickness =0;
#endif
  } else {
    event_box_gtab = gtk_event_box_new();
    gtk_event_box_set_visible_window (GTK_EVENT_BOX(event_box_gtab), FALSE);
  }

  gtk_container_set_border_width (GTK_CONTAINER (event_box_gtab), 0);

  if (gwin_gtab==NULL) create_win_gtab();

  if (hime_inner_frame) {
    GtkWidget *frame = top_bin = gtk_frame_new(NULL);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
    gtk_container_add (GTK_CONTAINER(gwin_gtab), frame);
    gtk_container_set_border_width (GTK_CONTAINER (gwin_gtab), 0);
    gtk_container_add (GTK_CONTAINER (frame), vbox_top);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  } else {
    gtk_container_add (GTK_CONTAINER(gwin_gtab), vbox_top);
    top_bin = vbox_top;
  }

  GtkWidget *hbox_edit = NULL;

  gboolean b_need_label_edit = need_label_edit();

  if (b_need_label_edit) {
    hbox_edit = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_edit, FALSE, FALSE, 0);
    GtkWidget *align_edit = gtk_alignment_new (0, 0.0, 0, 0);
    gtk_box_pack_start (GTK_BOX (hbox_edit), align_edit, FALSE, FALSE, 0);
    label_edit = gtk_label_new(NULL);
    gtk_box_pack_start (GTK_BOX (align_edit), label_edit, FALSE, FALSE, 0);
  }

  GtkWidget *align = gtk_alignment_new (0, 0.0, 0, 0);

  label_gtab_sele = gtk_label_new(NULL);
  gtk_container_add (GTK_CONTAINER (align), label_gtab_sele);

  if (!gtab_in_row1) {
    if (!gtab_vertical_select_on())
      gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);
  } else {
    GtkWidget *hbox_row1 = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row1, FALSE, FALSE, 0);

//    dbg("zzz %d zzz %d %d\n", b_need_label_edit, gtab_phrase_on(), hime_edit_display_ap_only());

    if (b_need_label_edit) {
      last_cursor_off = TRUE;
      gtk_box_pack_start (GTK_BOX (hbox_edit), event_box_gtab, FALSE, FALSE, 0);
    } else
      gtk_box_pack_start (GTK_BOX (hbox_row1), event_box_gtab, FALSE, FALSE, 0);

    if (!gtab_vertical_select_on())
      gtk_box_pack_start (GTK_BOX (hbox_row1), align, FALSE, FALSE, 0);
  }

  if (gtab_phrase_pre_select && !use_tsin_sel_win()) {
    label_gtab_pre_sel = gtk_label_new(NULL);
    set_label_font_size(label_gtab_pre_sel, hime_font_size_tsin_presel);
    gtk_box_pack_start (GTK_BOX (vbox_top), label_gtab_pre_sel, FALSE, FALSE, 0);
  }

  hbox_row2 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row2, FALSE, FALSE, 0);

  label_full = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_full), _(cht_full_str));

  gtk_box_pack_start (GTK_BOX (hbox_row2), label_full, FALSE, FALSE, 0);


  if (gtab_disp_im_name) {
    GtkWidget *event_box_input_method_name;
    if (gtab_in_area_button)
      event_box_input_method_name = gtk_button_new();
    else {
      event_box_input_method_name = gtk_event_box_new();
      gtk_event_box_set_visible_window (GTK_EVENT_BOX(event_box_input_method_name), FALSE);
    }

    gtk_box_pack_start (GTK_BOX (hbox_row2), event_box_input_method_name, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (event_box_input_method_name), 0);

    GtkWidget *frame_input_method_name = NULL;
    if (!gtab_in_area_button) {
    frame_input_method_name = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame_input_method_name), GTK_SHADOW_OUT);
    gtk_container_add (GTK_CONTAINER (event_box_input_method_name), frame_input_method_name);
    gtk_container_set_border_width (GTK_CONTAINER (frame_input_method_name), 0);
    }

    label_input_method_name = gtk_label_new("");
  //  dbg("gtk_label_new label_input_method_name\n");

    gtk_container_add (GTK_CONTAINER (gtab_in_area_button?event_box_input_method_name:frame_input_method_name), label_input_method_name);
    g_signal_connect_swapped (GTK_OBJECT (event_box_input_method_name), "button-press-event",
          G_CALLBACK (inmd_switch_popup_handler), NULL);

    box_gtab_im_name = event_box_input_method_name;
  }

  if (!gtab_in_row1)
    gtk_box_pack_start (GTK_BOX (hbox_row2), event_box_gtab, FALSE, FALSE, 0);


  if (!hime_display_on_the_spot_key()) {
    GtkWidget *frame_gtab = NULL;
    if (!gtab_in_area_button) {
      frame_gtab = gtk_frame_new(NULL);
      gtk_frame_set_shadow_type(GTK_FRAME(frame_gtab), GTK_SHADOW_OUT);
      gtk_container_set_border_width (GTK_CONTAINER (frame_gtab), 0);
      gtk_container_add (GTK_CONTAINER (event_box_gtab), frame_gtab);
    }
    g_signal_connect(G_OBJECT(event_box_gtab),"button-press-event",
                     G_CALLBACK(mouse_button_callback), NULL);

#if 0
    if (left_right_button_tips) {
#if GTK_CHECK_VERSION(2,12,0)
      gtk_widget_set_tooltip_text (event_box_gtab, _("左鍵符號，右鍵設定"));
#else
      GtkTooltips *button_gtab_tips = gtk_tooltips_new ();
      gtk_tooltips_set_tip (GTK_TOOLTIPS (button_gtab_tips), event_box_gtab, _("左鍵符號，右鍵設定"),NULL);
#endif
    }
#endif


    label_gtab = gtk_label_new(NULL);

    if (gtab_in_area_button)
      gtk_container_add (GTK_CONTAINER (event_box_gtab), label_gtab);
    else
      gtk_container_add (GTK_CONTAINER (frame_gtab), label_gtab);
  }

  label_key_codes  = gtk_label_new(NULL);
#if 0
  gtk_label_set_selectable(GTK_LABEL(label_key_codes), TRUE);
  mod_bg_all(label_key_codes, NULL);
#endif
  gtk_box_pack_start (GTK_BOX (hbox_row2), label_key_codes, FALSE, FALSE, 2);

  label_page  = gtk_label_new(NULL);
  gtk_box_pack_start (GTK_BOX (hbox_row2), label_page, FALSE, FALSE, 2);

  if (gtab_vertical_select_on()) {
    gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);
  }

  change_gtab_font_size();

  if ((current_method_type() != method_type_PHO) && (current_method_type()!=method_type_PHO))
    gtk_widget_show_all (gwin_gtab);
  gtk_widget_hide (gwin_gtab);
  gtk_widget_hide(label_gtab_sele);
  gtk_widget_hide(label_key_codes);
  gtk_widget_hide(label_page);
  if (GTK_IS_WIDGET(label_gtab_pre_sel))
    gtk_widget_hide(label_gtab_pre_sel);

  show_hide_label_edit();

  set_disp_im_name();
  gtk_widget_hide(label_full);

  if (gtab_hide_row2)
    gtk_widget_hide(hbox_row2);
}

void disp_gtab_pre_sel(char *s)
{
//  dbg("disp_gtab_pre_sel %s\n", s);
  if (!label_gtab_pre_sel)
    show_win_gtab();

//  dbg("label_gtab_pre_sel %x %d\n", label_gtab_pre_sel, use_tsin_sel_win());
  gtk_widget_show(label_gtab_pre_sel);
  if (s) gtk_label_set_markup(GTK_LABEL(label_gtab_pre_sel), s);
  show_win_gtab();
  adj_gtab_win_pos();
}

void show_input_method_name(GtkWidget *label, gchar *cname)
{
  if (hime_win_color_use)
  {
    gchar *color_cname = g_strdup_printf("<span foreground=\"%s\">[%s]</span>",
				         hime_sel_key_color, cname);
    gtk_label_set_markup(GTK_LABEL(label), color_cname);
    g_free(color_cname);
  }
  else
    gtk_label_set_text(GTK_LABEL(label), cname);
}

GtkWidget *get_cname_label();

void show_input_method_name_on_gtab()
{
  if (current_CS && (! hime_status_tray) && gtab_disp_im_name &&
      (current_CS->im_state == HIME_STATE_CHINESE) && (current_CS->b_half_full_char == 0) &&
      (current_CS->tsin_pho_mode))
  {
    if ((current_method_type() == method_type_MODULE) ||
        (gtab_hide_row2 && (hime_edit_display == HIME_EDIT_DISPLAY_ON_THE_SPOT)))
    // label_gtab = NULL under onthespot mode.
    {
      if (current_method_type() == method_type_MODULE)
      {
        show_win_gtab();
	adj_gtab_win_pos();
        if (hbox_row2) gtk_widget_hide(hbox_row2);
      }

      if (label_gtab == NULL) create_win_gtab_gui_simple();
      disp_gtab_sel(NULL);
      show_input_method_name(label_gtab_sele, inmd[current_CS->in_method].cname);
      return;
    }

    if ((label_gtab==NULL) || (! gtab_hide_row2)) return;

    show_input_method_name(label_gtab, inmd[current_CS->in_method].cname);
  }
}

static void create_win_gtab_gui()
{
  create_win_gtab_gui_simple();
  current_gtab_in_row1 = gtab_in_row1;
  current_gtab_vertical_select = gtab_vertical_select_on();
  current_hime_inner_frame = hime_inner_frame;
  gdk_color_parse("red", &better_color);
}


void change_win_gtab_style()
{
  destroy_if_necessary();
  create_win_gtab_gui();
}


void init_gtab(int inmdno);
gboolean gtab_has_input();
extern gboolean force_show;

void show_win_gtab()
{
  create_win_gtab();
  create_win_gtab_gui();
  // window was destroyed
  if (hime_pop_up_win)
    set_key_codes_label(str_key_codes, better_key_codes);

  if (current_CS) {
    if (current_CS->fixed_pos)
      move_win_gtab(0,0);
  }

//  init_gtab(current_CS->in_method);

  if (hime_pop_up_win && !gtab_has_input() &&
      !force_show && poo.same_pho_query_state==SAME_PHO_QUERY_none && !tss.pre_selN)
    return;

//  dbg("show_win_gtab()\n");

#if 0
  if (current_CS->b_raise_window)
#endif
    gtk_window_present(GTK_WINDOW(gwin_gtab));

  move_win_gtab(current_in_win_x, current_in_win_y);

  if ((current_method_type() != method_type_PHO) && (current_method_type()!=method_type_PHO))
    gtk_widget_show(gwin_gtab);

  if (current_CS)
  {
    if (current_CS->tsin_pho_mode==0)
      set_gtab_input_method_name(eng_half_str);
    else
      set_gtab_input_method_name(inmd[current_CS->in_method].cname);

    if ((GTK_IS_WIDGET (label_gtab)) && (hime_status_tray || (! gtab_hide_row2)))
      gtk_widget_hide(label_gtab);
    
    win_gtab_disp_half_full();
  }

  show_win_sym();
}


void close_gtab_pho_win();

static void destroy_top_bin()
{
  if (!top_bin)
    return;
  gtk_widget_destroy(top_bin);
  top_bin = NULL;
  hbox_row2 = NULL;
  label_full=NULL;
  label_gtab_sele = NULL;
  label_gtab = NULL;
  label_input_method_name = NULL;
  label_key_codes = NULL;
  box_gtab_im_name = NULL;
  label_page = NULL;
  label_edit = NULL;
  label_gtab_pre_sel = NULL;
}

void destroy_win_gtab()
{
  if (!gwin_gtab)
    return;
  destroy_top_bin();
  gtk_widget_destroy(gwin_gtab);
  gwin_gtab = NULL;
}

void hide_win_kbm();

void hide_win_gtab()
{
  win_gtab_max_key_press = 0;

  if (!gwin_gtab)
    return;

//  dbg("hide_win_gtab\n");
  if (gwin_gtab) {
    if (destroy_window)
      destroy_win_gtab();
    else {
      gtk_widget_hide(gwin_gtab);
      destroy_top_bin();
    }
  }

  close_gtab_pho_win();
  hide_win_sym();
  hide_win_kbm();
}

void get_win_gtab_geom()
{
  if (!gwin_gtab)
    return;

  gtk_window_get_position(GTK_WINDOW(gwin_gtab), &win_x, &win_y);

  get_win_size(gwin_gtab, &win_xl, &win_yl);
}

static void set_disp_im_name()
{
  if (!box_gtab_im_name)
    return;

  if (gtab_disp_im_name)
    gtk_widget_show(box_gtab_im_name);
  else
    gtk_widget_hide(box_gtab_im_name);
}

char *get_full_str()
{
  if (current_CS->tsin_pho_mode==0)
  {
    if (hime_win_color_use)
      return eng_color_half_str;
    else
      return _(eng_half_str);
  }

  switch (current_CS->im_state) {
    case HIME_STATE_CHINESE:
      if (current_CS->b_half_full_char)
      {
        if (hime_win_color_use)
          return cht_color_full_str;
        else
          return _(cht_full_str);
      }
      break;
    case HIME_STATE_ENG_FULL:
      if (hime_win_color_use)
        return eng_color_full_str;
      else
        return _(eng_full_str);
      break;
    default:
      break;
  }
  return ("");
}

void win_gtab_disp_half_full()
{
  if (!gwin_gtab)
    return;
  if (label_full) {
    if ((current_CS->im_state == HIME_STATE_CHINESE && (!current_CS->b_half_full_char)) ||
        (current_CS->tsin_pho_mode == 0))
      gtk_widget_hide(label_full);
    else
      gtk_widget_show(label_full);
  }

  if (current_CS->tsin_pho_mode)
  {
    if (label_gtab_sele) gtk_widget_show(label_gtab_sele);
    if (hime_status_tray || (! gtab_hide_row2))
      if (GTK_IS_WIDGET(label_gtab)) gtk_widget_show(label_gtab);
  }
  else
  {
    if (label_gtab_sele) gtk_widget_hide(label_gtab_sele);
    if (hime_status_tray || (! gtab_hide_row2))
      if (label_gtab) gtk_widget_hide(label_gtab);
  }

  if (gtab_hide_row2)
  {
    if (GTK_IS_WIDGET(label_gtab))
    {
      if (hime_win_color_use) gtk_label_set_markup(GTK_LABEL(label_gtab), get_full_str());
      else  gtk_label_set_text(GTK_LABEL(label_gtab), get_full_str());
    }
    else
    {
      gchar *full_str = get_full_str();
      if (full_str && full_str[0])
        disp_gtab_sel(full_str);
      else
        disp_gtab_sel(inmd[current_CS->in_method].cname);
    }
  }
}

void hide_selections_win();

void hide_gtab_pre_sel()
{
  if (use_tsin_sel_win())
    hide_selections_win();

//  dbg("hide_gtab_pre_sel %d\n", tss.ctrl_pre_sel);
  tss.pre_selN = 0;
  tss.ctrl_pre_sel = FALSE;
  if (label_gtab_pre_sel)
    gtk_widget_hide(label_gtab_pre_sel);

  move_win_gtab(current_in_win_x, current_in_win_y);
  adj_gtab_win_pos();
}
