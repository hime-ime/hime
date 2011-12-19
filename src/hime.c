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
#include "config.h"
#include "hime-version.h"
#include "gtab.h"
#if UNIX
#include <signal.h>
#endif
#if HIME_i18n_message
#include <libintl.h>
#endif

#if UNIX
Window root;
#endif
Display *dpy;

int win_xl, win_yl;
int win_x, win_y;   // actual win x/y
int dpy_xl, dpy_yl;
DUAL_XIM_ENTRY xim_arr[1];

extern unich_t *fullchar[];
gboolean win_kbm_inited;

char *half_char_to_full_char(KeySym xkey)
{
  if (xkey < ' ' || xkey > 127)
    return NULL;
  return _(fullchar[xkey-' ']);
}


#if UNIX
static void start_inmd_window()
{
  GtkWidget *win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_realize (win);
  GdkWindow *gdkwin0 = gtk_widget_get_window(win);
  xim_arr[0].xim_xwin = GDK_WINDOW_XWINDOW(gdkwin0);
  dbg("xim_xwin %x\n", xim_arr[0].xim_xwin);
}
#endif


#if USE_XIM
char *lc;

static XIMStyle Styles[] = {
#if 1
        XIMPreeditCallbacks|XIMStatusCallbacks,         //OnTheSpot
        XIMPreeditCallbacks|XIMStatusArea,              //OnTheSpot
        XIMPreeditCallbacks|XIMStatusNothing,           //OnTheSpot
#endif
        XIMPreeditPosition|XIMStatusArea,               //OverTheSpot
        XIMPreeditPosition|XIMStatusNothing,            //OverTheSpot
        XIMPreeditPosition|XIMStatusNone,               //OverTheSpot
#if 1
        XIMPreeditArea|XIMStatusArea,                   //OffTheSpot
        XIMPreeditArea|XIMStatusNothing,                //OffTheSpot
        XIMPreeditArea|XIMStatusNone,                   //OffTheSpot
#endif
        XIMPreeditNothing|XIMStatusNothing,             //Root
        XIMPreeditNothing|XIMStatusNone,                //Root
};
static XIMStyles im_styles;

#if 1
static XIMTriggerKey trigger_keys[] = {
        {XK_space, ControlMask, ControlMask},
        {XK_space, ShiftMask, ShiftMask},
        {XK_space, Mod1Mask, Mod1Mask},   // Alt
        {XK_space, Mod4Mask, Mod4Mask},   // Windows
};
#endif

/* Supported Encodings */
static XIMEncoding chEncodings[] = {
        "COMPOUND_TEXT",
        0
};
static XIMEncodings encodings;

int xim_ForwardEventHandler(IMForwardEventStruct *call_data);

XIMS current_ims;
extern void toggle_im_enabled();


int MyTriggerNotifyHandler(IMTriggerNotifyStruct *call_data)
{
//    dbg("MyTriggerNotifyHandler %d %x\n", call_data->key_index, call_data->event_mask);

    if (call_data->flag == 0) { /* on key */
//        db(g("trigger %d\n", call_data->key_index);
        if ((call_data->key_index == 0 && hime_im_toggle_keys==Control_Space) ||
            (call_data->key_index == 3 && hime_im_toggle_keys==Shift_Space) ||
            (call_data->key_index == 6 && hime_im_toggle_keys==Alt_Space) ||
            (call_data->key_index == 9 && hime_im_toggle_keys==Windows_Space)
            ) {
            toggle_im_enabled();
        }
        return True;
    } else {
        /* never happens */
        return False;
    }
}

#if 0
void switch_IC_index(int index);
#endif
void CreateIC(IMChangeICStruct *call_data);
void DeleteIC(CARD16 icid);
void SetIC(IMChangeICStruct * call_data);
void GetIC(IMChangeICStruct *call_data);
int xim_hime_FocusIn(IMChangeFocusStruct *call_data);
int xim_hime_FocusOut(IMChangeFocusStruct *call_data);

int hime_ProtoHandler(XIMS ims, IMProtocol *call_data)
{
//  dbg("hime_ProtoHandler %x ims\n", ims);

  current_ims = ims;

  switch (call_data->major_code) {
  case XIM_OPEN:
#define MAX_CONNECT 20000
    {
      IMOpenStruct *pimopen=(IMOpenStruct *)call_data;

      if(pimopen->connect_id > MAX_CONNECT - 1)
        return True;

#if DEBUG && 0
    dbg("open lang %s  connectid:%d\n", pimopen->lang.name, pimopen->connect_id);
#endif
      return True;
    }
  case XIM_CLOSE:
#if DEBUG && 0
    dbg("XIM_CLOSE\n");
#endif
    return True;
  case XIM_CREATE_IC:
#if DEBUG && 0
     dbg("CREATE_IC\n");
#endif
     CreateIC((IMChangeICStruct *)call_data);
     return True;
  case XIM_DESTROY_IC:
     {
       IMChangeICStruct *pimcha=(IMChangeICStruct *)call_data;
#if DEBUG && 0
       dbg("DESTROY_IC %d\n", pimcha->icid);
#endif
       DeleteIC(pimcha->icid);
     }
     return True;
  case XIM_SET_IC_VALUES:
#if DEBUG && 0
     dbg("SET_IC\n");
#endif
     SetIC((IMChangeICStruct *)call_data);
     return True;
  case XIM_GET_IC_VALUES:
#if DEBUG && 0
     dbg("GET_IC\n");
#endif
     GetIC((IMChangeICStruct *)call_data);
     return True;
  case XIM_FORWARD_EVENT:
#if DEBUG && 0
     dbg("XIM_FORWARD_EVENT\n");
#endif
     return xim_ForwardEventHandler((IMForwardEventStruct *)call_data);
  case XIM_SET_IC_FOCUS:
#if DEBUG && 0
     dbg("XIM_SET_IC_FOCUS\n");
#endif
     return xim_hime_FocusIn((IMChangeFocusStruct *)call_data);
  case XIM_UNSET_IC_FOCUS:
#if DEBUG && 0
     dbg("XIM_UNSET_IC_FOCUS\n");
#endif
     return xim_hime_FocusOut((IMChangeFocusStruct *)call_data);
  case XIM_RESET_IC:
#if DEBUG && 0
     dbg("XIM_UNSET_IC_FOCUS\n");
#endif
     return True;
  case XIM_TRIGGER_NOTIFY:
#if DEBUG && 0
     dbg("XIM_TRIGGER_NOTIFY\n");
#endif
     MyTriggerNotifyHandler((IMTriggerNotifyStruct *)call_data);
     return True;
  case XIM_PREEDIT_START_REPLY:
#if DEBUG && 1
     dbg("XIM_PREEDIT_START_REPLY\n");
#endif
     return True;
  case XIM_PREEDIT_CARET_REPLY:
#if DEBUG && 1
     dbg("XIM_PREEDIT_CARET_REPLY\n");
#endif
     return True;
  case XIM_STR_CONVERSION_REPLY:
#if DEBUG && 1
     dbg("XIM_STR_CONVERSION_REPLY\n");
#endif
     return True;
  default:
     printf("Unknown major code.\n");
     break;
  }

  return True;
}


void open_xim()
{
  XIMTriggerKeys triggerKeys;

  im_styles.supported_styles = Styles;
  im_styles.count_styles = sizeof(Styles)/sizeof(Styles[0]);

  triggerKeys.count_keys = sizeof(trigger_keys)/sizeof(trigger_keys[0]);
  triggerKeys.keylist = trigger_keys;

  encodings.count_encodings = sizeof(chEncodings)/sizeof(XIMEncoding) - 1;
  encodings.supported_encodings = chEncodings;

  if ((xim_arr[0].xims = IMOpenIM(dpy,
          IMServerWindow,         xim_arr[0].xim_xwin,        //input window
          IMModifiers,            "Xi18n",        //X11R6 protocol
          IMServerName,           xim_arr[0].xim_server_name, //XIM server name
          IMLocale,               lc,
          IMServerTransport,      "X/",      //Comm. protocol
          IMInputStyles,          &im_styles,   //faked styles
          IMEncodingList,         &encodings,
          IMProtocolHandler,      hime_ProtoHandler,
          IMFilterEventMask,      KeyPressMask|KeyReleaseMask,
          IMOnKeysList, &triggerKeys,
          NULL)) == NULL) {
          setenv("NO_GTK_INIT", NULL, TRUE);
          p_err("IMOpenIM '%s' failed. Maybe another XIM server is running.\n",
          xim_arr[0].xim_server_name);
  }
}

#endif // if USE_XIM

void load_tsin_db();
void load_tsin_conf(), load_setttings(), load_tab_pho_file();

void disp_hide_tsin_status_row(), gcb_main(), update_win_kbm_inited();
void change_tsin_line_color(), change_win0_style(), change_tsin_color();
void change_win_gtab_style();
void update_item_active_all();
void destroy_inmd_menu();
void load_gtab_list(gboolean);
void change_win1_font();
void set_wselkey(char *s);

static void reload_data()
{
  dbg("reload_data\n");
  load_setttings();
  if (current_method_type()==method_type_TSIN)
    set_wselkey(pho_selkey);

//  load_tsin_db();
  change_win0_style();
  change_win1_font();
  change_win_gtab_style();
//  change_win_pho_style();
  load_tab_pho_file();
  change_tsin_color();
  update_win_kbm_inited();

  destroy_inmd_menu();
  load_gtab_list(TRUE);

  update_item_active_all();
#if 0
  gcb_main();
#endif
}

void change_tsin_font_size();
void change_gtab_font_size();
void change_pho_font_size();
void change_win_sym_font_size();
void change_win_gtab_style();
extern int win_kbm_on;

static void change_font_size()
{
  load_setttings();
  change_tsin_font_size();
  change_gtab_font_size();
  change_pho_font_size();
  change_win_sym_font_size();
  change_win0_style();
  change_win_gtab_style();
  update_win_kbm_inited();
//  change_win_pho_style();
}

#if UNIX
static int xerror_handler(Display *d, XErrorEvent *eve)
{
  return 0;
}
#endif

#if UNIX
Atom hime_atom;
#endif
void disp_tray_icon(), toggle_gb_output();

void cb_trad_sim_toggle()
{
  toggle_gb_output();
#if TRAY_ENABLED
  disp_tray_icon();
#endif
}
void execute_message(char *message), show_win_kbm(), hide_win_kbm();
int b_show_win_kbm=0;
void disp_win_kbm_capslock_init();

void kbm_open_close(gboolean b_show)
{
  b_show_win_kbm=b_show;
  if (b_show) {
    show_win_kbm();
    disp_win_kbm_capslock_init();
  } else
    hide_win_kbm();
}

void kbm_toggle()
{
  win_kbm_inited = 1;
  kbm_open_close(!b_show_win_kbm);
}


void reload_tsin_db();
void do_exit();

void message_cb(char *message)
{
   void sim_output();  // FIXME
   void trad_output(); // FIXME
//   dbg("message '%s'\n", message);

   /* TODO: rewrite the mess with case() ? */
   if (!strcmp(message, CHANGE_FONT_SIZE)) {
     change_font_size();
   } else
   if (!strcmp(message, GB_OUTPUT_TOGGLE)) {
     cb_trad_sim_toggle();
     update_item_active_all();
   } else
   if (!strcmp(message, SIM_OUTPUT_TOGGLE)) {
     sim_output();
#if TRAY_ENABLED
     disp_tray_icon();
#endif
     update_item_active_all();
   } else
   if (!strcmp(message, TRAD_OUTPUT_TOGGLE)) {
     trad_output();
#if TRAY_ENABLED
     disp_tray_icon();
#endif
     update_item_active_all();
   } else
   if (!strcmp(message, KBM_TOGGLE)) {
     kbm_toggle();
   } else
#if UNIX
   if (strstr(message, "#hime_message")) {
     execute_message(message);
   } else
#endif
   if (!strcmp(message, RELOAD_TSIN_DB)) {
     reload_tsin_db();
   } else
   if (!strcmp(message, HIME_EXIT_MESSAGE)) {
     do_exit();
   } else
     reload_data();
}

#if UNIX
static GdkFilterReturn my_gdk_filter(GdkXEvent *xevent,
                                     GdkEvent *event,
                                     gpointer data)
{
   XEvent *xeve = (XEvent *)xevent;
#if 0
   dbg("a zzz %d\n", xeve->type);
#endif

   // only very old WM will enter this
   if (xeve->type == FocusIn || xeve->type == FocusOut) {
#if 0
     dbg("focus %s\n", xeve->type == FocusIn ? "in":"out");
#endif
     return GDK_FILTER_REMOVE;
   }

#if USE_XIM
   if (XFilterEvent(xeve, None) == True)
     return GDK_FILTER_REMOVE;
#endif

   return GDK_FILTER_CONTINUE;
}

void init_atom_property()
{
  hime_atom = get_hime_atom(dpy);
  XSetSelectionOwner(dpy, hime_atom, xim_arr[0].xim_xwin, CurrentTime);
}
#endif


void hide_win0();
void destroy_win0();
void destroy_win1();
void destroy_win_gtab();
void free_pho_mem(),free_tsin(),free_all_IC(), free_gtab(), free_phrase(), destroy_tray_win32();

void do_exit()
{
  dbg("----------------- do_ exit ----------------\n");

  free_pho_mem();
  free_tsin();
#if USE_XIM
  free_all_IC();
#endif
  free_gtab();
  free_phrase();

#if 1
  destroy_win0();
  destroy_win1();
  destroy_win_gtab();
#endif

#if WIN32
  destroy_tray_win32();
#endif
  gtk_main_quit();
}

void sig_do_exit(int sig)
{
  do_exit();
}

char *get_hime_xim_name();
void load_phrase(), init_TableDir();
void init_tray(), exec_setup_scripts();
#if UNIX
void init_hime_im_serv(Window win);
#else
void init_hime_im_serv();
#endif
void gcb_main(), init_tray_win32();

#if WIN32
void init_hime_program_files();
 #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif
int win32_tray_disabled = 1;


gboolean delayed_start_cb(gpointer data)
{
#if WIN32
  Sleep(200);
#endif

  win32_tray_disabled = 0;

#if TRAY_ENABLED
  if (hime_status_tray) {
#if UNIX
    if (hime_win32_icon)
      init_tray_win32();
    else
      init_tray();
#endif
  }
#endif

  dbg("after init_tray\n");

#if 0
  if (gcb_position)
    gcb_main();

  dbg("after gcb_main\n");
#endif

  return FALSE;
}

void get_dpy_xyl()
{
	dpy_xl = gdk_screen_width(), dpy_yl = gdk_screen_height();
}

void screen_size_changed(GdkScreen *screen, gpointer user_data)
{
	get_dpy_xyl();
}

#include "lang.h"

extern int destroy_window;

int main(int argc, char **argv)
{
#if WIN32
   putenv("PANGO_WIN32_NO_UNISCRIBE=1");
#endif

  char *destroy = getenv("HIME_DESTROY_WINDOW");
  if (destroy)
    destroy_window = atoi(destroy);
//  printf("HIME_DESTROY_WINDOW=%d\n",destroy_window);

  gtk_init (&argc, &argv);

#if GTK_CHECK_VERSION(2,91,6)
  static char css[]=
"GtkButton\n"
"{\n"
"  border-width: 0;\n"
"  padding: 0;\n"
"  -GtkButton-inner-border: 0;\n"
"}";
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_data(provider, css, -1, NULL);
  gtk_style_context_add_provider_for_screen(gdk_display_get_default_screen(gdk_display_get_default()), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
#else
static char button_rc[]="style \"button\"\n"
"{\n"
"   GtkButton::inner-border = {0,0,0,0}\n"
"\n"
"xthickness = 1\n"
"ythickness = 0\n"
"}\n"
"class \"GtkButton\" style \"button\"";
  gtk_rc_parse_string(button_rc);
#endif

#if UNIX
  signal(SIGCHLD, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  if (getenv("HIME_DAEMON")) {
    daemon(1,1);
#if FREEBSD
    setpgid(0, getpid());
#else
    setpgrp();
#endif
  }
#endif

//putenv("GDK_NATIVE_WINDOWS=1");
#if WIN32
  typedef BOOL (WINAPI* pImmDisableIME)(DWORD);
  pImmDisableIME pd;
  HMODULE imm32=LoadLibraryA("imm32");
  if (imm32 && (pd=(pImmDisableIME)GetProcAddress(imm32, "ImmDisableIME"))) {
     (*pd)(0);
  }
  init_hime_program_files();
  init_hime_im_serv();
#endif

  set_is_chs();

#if UNIX
  char *lc_ctype = getenv("LC_CTYPE");
  char *lc_all = getenv("LC_ALL");
  char *lang = getenv("LANG");
  if (!lc_ctype && lang)
    lc_ctype = lang;

  if (lc_all)
    lc_ctype = lc_all;

  if (!lc_ctype)
    lc_ctype = "zh_TW.Big5";
  dbg("hime get env LC_CTYPE=%s  LC_ALL=%s  LANG=%s\n", lc_ctype, lc_all, lang);
#endif

#if USE_XIM
  char *t = strchr(lc_ctype, '.');
  if (t) {
    int len = t - lc_ctype;
#if MAC_OS || FREEBSD
    lc = strdup(lc_ctype);
    lc[len] = 0;
#else
    lc = g_strndup(lc_ctype, len);
#endif
  }
  else
    lc = lc_ctype;

  char *xim_server_name = get_hime_xim_name();

  strcpy(xim_arr[0].xim_server_name, xim_server_name);

  dbg("hime XIM will use %s as the default encoding\n", lc_ctype);
#endif

  if (argc == 2 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version") || !strcmp(argv[1], "-h")) ) {
    setenv("NO_GTK_INIT", "", TRUE);
    p_err(" version %s\n", HIME_VERSION);
  }

  init_TableDir();
  load_setttings();
  load_gtab_list(TRUE);


#if HIME_i18n_message
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif

  dbg("after gtk_init\n");

#if UNIX
  dpy = GDK_DISPLAY();
  root=DefaultRootWindow(dpy);
#endif
  get_dpy_xyl();
  g_signal_connect(gdk_screen_get_default(),"size-changed", G_CALLBACK(screen_size_changed), NULL);

  dbg("display width:%d height:%d\n", dpy_xl, dpy_yl);

#if UNIX
  start_inmd_window();
#endif

#if USE_XIM
  open_xim();
#endif

#if UNIX
  gdk_window_add_filter(NULL, my_gdk_filter, NULL);

  init_atom_property();
  signal(SIGINT, sig_do_exit);
  signal(SIGHUP, sig_do_exit);
  // disable the io handler abort
  // void *olderr =
    XSetErrorHandler((XErrorHandler)xerror_handler);
#endif

#if UNIX
  init_hime_im_serv(xim_arr[0].xim_xwin);
#endif

  exec_setup_scripts();

#if UNIX
  g_timeout_add(5000, delayed_start_cb, NULL);
#else
  delayed_start_cb(NULL);
#endif

  dbg("before gtk_main\n");

  disp_win_kbm_capslock_init();

  gtk_main();

  return 0;
}
