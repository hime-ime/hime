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

#include <sys/stat.h>
#include <X11/extensions/XTest.h>
#include "hime.h"
#include "gtab.h"
extern INMD *cur_inmd;

static GtkWidget *gwin_kbm;
#if !GTK_CHECK_VERSION(2,91,6)
static GdkColor red;
#else
static GdkRGBA red;
#endif
gboolean win_kbm_on = FALSE;
extern gboolean test_mode;

enum {
  K_FILL=1,
  K_HOLD=2,
  K_PRESS=4,
  K_AREA_R=8,
  K_CAPSLOCK=16
};


typedef struct {
  KeySym keysym;
  unich_t *enkey;
  char shift_key;
  char flag;
  GtkWidget *lab, *but, *laben;
} KEY;

#if TRAY_ENABLED
extern void update_item_active_all();
#endif

/* include win-kbm.h here so we do not have to translate those N_("stuff") */
#include "win-kbm.h"

static int keysN=sizeof(keys)/sizeof(keys[0]);

void update_win_kbm();

#if !GTK_CHECK_VERSION(2,91,6)
void mod_fg_all(GtkWidget *lab, GdkColor *col)
{
  if (lab==NULL) return;
  gtk_widget_modify_fg(lab, GTK_STATE_NORMAL, col);
  gtk_widget_modify_fg(lab, GTK_STATE_ACTIVE, col);
  gtk_widget_modify_fg(lab, GTK_STATE_SELECTED, col);
  gtk_widget_modify_fg(lab, GTK_STATE_PRELIGHT, col);
}
#else
void mod_fg_all(GtkWidget *lab, GdkRGBA *rgbfg)
{
  gtk_widget_override_color(lab, GTK_STATE_FLAG_NORMAL, rgbfg);
  gtk_widget_override_color(lab, GTK_STATE_FLAG_ACTIVE, rgbfg);
  gtk_widget_override_color(lab, GTK_STATE_FLAG_SELECTED, rgbfg);
  gtk_widget_override_color(lab, GTK_STATE_FLAG_PRELIGHT, rgbfg);
}
#endif

void send_fake_key_eve(KeySym key);

void send_fake_key_eve2(KeySym key, gboolean press)
{
  KeyCode kc = XKeysymToKeycode(dpy, key);
  XTestFakeKeyEvent(dpy, kc, press, CurrentTime);
}

static int kbm_timeout_handle;

static gboolean timeout_repeat(gpointer data)
{
  KeySym k = GPOINTER_TO_INT(data);

  send_fake_key_eve2(k, TRUE);
  return TRUE;
}

static gboolean timeout_first_time(gpointer data)
{
  KeySym k = GPOINTER_TO_INT(data);
  dbg("timeout_first_time %c\n", k);
  send_fake_key_eve2(k, TRUE);
  kbm_timeout_handle = g_timeout_add(50, timeout_repeat, data);
  return FALSE;
}

static void clear_hold(KEY *k)
{
  KeySym keysym=k->keysym;
  GtkWidget *laben = k->laben;
  k->flag &= ~K_PRESS;
  mod_fg_all(laben, NULL);
  send_fake_key_eve2(keysym, FALSE);
}

static gboolean timeout_clear_hold(gpointer data)
{
  clear_hold((KEY *)data);
  return FALSE;
}

void clear_kbm_timeout_handle()
{
  if (!kbm_timeout_handle)
    return;
  g_source_remove(kbm_timeout_handle);
  kbm_timeout_handle = 0;
}

static void cb_button_click(GtkWidget *wid, KEY *k)
{
  KeySym keysym=k->keysym;
  GtkWidget *laben = k->laben;

  dbg("cb_button_click keysym %d\n", keysym);

  if (k->flag & K_HOLD) {
    if (k->flag & K_PRESS) {
      clear_hold(k);
    } else {
      send_fake_key_eve2(keysym, TRUE);
      k->flag |= K_PRESS;
      mod_fg_all(laben, &red);
      g_timeout_add(10000, timeout_clear_hold, GINT_TO_POINTER(k));
    }
  } else {
	clear_kbm_timeout_handle();
    kbm_timeout_handle = g_timeout_add(500, timeout_first_time, GINT_TO_POINTER(keysym));
    send_fake_key_eve2(keysym, TRUE);
  }
}


static void cb_button_release(GtkWidget *wid, KEY *k)
{
    dbg("cb_button_release %d\n", kbm_timeout_handle);
	clear_kbm_timeout_handle();

    send_fake_key_eve2(k->keysym, FALSE);

    int i;
    for(i=0;i<keysN;i++) {
      int j;
      for(j=0; keys[i][j].enkey; j++) {
        if (!(keys[i][j].flag & K_PRESS))
          continue;
        keys[i][j].flag &= ~K_PRESS;
                send_fake_key_eve2(keys[i][j].keysym, FALSE);
        mod_fg_all(keys[i][j].laben, NULL);
      }
    }
}


static void create_win_kbm()
{
#if !GTK_CHECK_VERSION(2,91,6)
  gdk_color_parse("red", &red);
#else
  gdk_rgba_parse(&red, "red");
#endif

  gwin_kbm = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(gwin_kbm), FALSE);

  gtk_container_set_border_width (GTK_CONTAINER (gwin_kbm), 0);
  GtkWidget *hbox_top = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gwin_kbm), hbox_top);


  GtkWidget *vbox_l = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_l), GTK_ORIENTATION_VERTICAL);
  gtk_box_pack_start (GTK_BOX (hbox_top), vbox_l, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_l), 0);
  GtkWidget *vbox_r = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_r), GTK_ORIENTATION_VERTICAL);
  gtk_box_pack_start (GTK_BOX (hbox_top), vbox_r, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_r), 0);

  int i;
  for(i=0;i<keysN;i++) {
    GtkWidget *hboxl = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hboxl), 0);
    gtk_box_pack_start (GTK_BOX (vbox_l), hboxl, FALSE, FALSE, 0);
    GtkWidget *hboxr = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hboxr), 0);
    gtk_box_pack_start (GTK_BOX (vbox_r), hboxr, FALSE, FALSE, 0);
    KEY *pk = keys[i];

    int j;
    for(j=0; pk[j].enkey; j++) {
      KEY *ppk=&pk[j];
      char flag=ppk->flag;
      if (!ppk->keysym)
        continue;
      GtkWidget *but=pk[j].but=gtk_button_new();
      g_signal_connect (G_OBJECT (but), "pressed", G_CALLBACK (cb_button_click), ppk);
      if (!(ppk->flag & K_HOLD))
        g_signal_connect (G_OBJECT (but), "released", G_CALLBACK (cb_button_release), ppk);

      GtkWidget *hbox = (flag&K_AREA_R)?hboxr:hboxl;

      gtk_container_set_border_width (GTK_CONTAINER (but), 0);

      if (flag & K_FILL)
        gtk_box_pack_start (GTK_BOX (hbox), but, TRUE, TRUE, 0);
      else
        gtk_box_pack_start (GTK_BOX (hbox), but, FALSE, FALSE, 0);

      GtkWidget *v = gtk_vbox_new (FALSE, 0);
      gtk_orientable_set_orientation(GTK_ORIENTABLE(v), GTK_ORIENTATION_VERTICAL);
      gtk_container_set_border_width (GTK_CONTAINER (v), 0);
      gtk_container_add (GTK_CONTAINER (but), v);
      GtkWidget *laben = ppk->laben=gtk_label_new(_(ppk->enkey));
      set_label_font_size(laben, hime_font_size_win_kbm_en);
      gtk_box_pack_start (GTK_BOX (v), laben, FALSE, FALSE, 0);

      if (i>0&&i<5) {
        GtkWidget *lab = ppk->lab = gtk_label_new("  ");
//        set_label_font_size(lab, hime_font_size_win_kbm);
        gtk_box_pack_start (GTK_BOX (v), lab, FALSE, FALSE, 0);
      }
    }
  }

  gtk_widget_realize (gwin_kbm);
  set_no_focus(gwin_kbm);
}

#if TRAY_ENABLED
extern GtkStatusIcon *tray_icon;
extern GtkStatusIcon *icon_main;

extern gboolean is_exist_tray();
extern gboolean is_exist_tray_double();
#endif

static void move_win_kbm()
{
  int width, height;
  get_win_size(gwin_kbm, &width, &height);

  int ox, oy;
  GdkRectangle r;
  GtkOrientation ori;

#if TRAY_ENABLED
  if ((is_exist_tray() && gtk_status_icon_get_geometry(tray_icon, NULL, &r,  &ori)) || (is_exist_tray_double() && gtk_status_icon_get_geometry(icon_main, NULL, &r,  &ori))) {
//    dbg("rect %d:%d %d:%d\n", r.x, r.y, r.width, r.height);
    ox = r.x;
    if (ox + width > dpy_xl)
      ox = dpy_xl - width;

    if (r.y < 100)
      oy=r.y+r.height;
    else {
      oy = r.y - height;
    }
  } else
#endif
  {
    ox = dpy_xl - width;
    oy = dpy_yl - height - 16;
  }

  gtk_window_move(GTK_WINDOW(gwin_kbm), ox, oy);
}

void show_win_kbm()
{
  if (!gwin_kbm) {
    create_win_kbm();
    update_win_kbm();
  }

  gtk_widget_show_all(gwin_kbm);
  win_kbm_on = TRUE;
#if TRAY_ENABLED
  update_item_active_all();
#endif
  move_win_kbm();
}

static char   shift_chars[]="~!@#$%^&*()_+{}|:\"<>?";
static char shift_chars_o[]="`1234567890-=[]\\;',./";

#include "pho.h"

static KEY *get_keys_ent(KeySym keysym)
{
  int i;
  for(i=0;i<keysN;i++) {
    int j;
    for(j=0;j<COLN;j++) {
      char *p;
      if (keysym >='A' && keysym<='Z')
        keysym += 0x20;
      else
      if ((p=strchr(shift_chars, keysym))) {
        keysym = shift_chars_o[p - shift_chars];
      }

      if (keys[i][j].keysym!=keysym)
        continue;
      return &keys[i][j];
    }
  }

  return NULL;
}

static void set_kbm_key(KeySym keysym, char *str)
{
  if (!gwin_kbm)
    return;
#if 0
  if (strlen(str)==1 && !(str[0] & 0x80))
    return;
#endif

  KEY *p = get_keys_ent(keysym);
  if (!p)
    return;

  GtkWidget *lab = p->lab;
  char *t = (char *)gtk_label_get_text(GTK_LABEL(lab));
  char tt[64];

  if (t && strcmp(t, str)) {
    strcat(strcpy(tt, t), str);
    str = tt;
  }

  if (lab) {
    gtk_label_set_text(GTK_LABEL(lab), str);
    set_label_font_size(lab, hime_font_size_win_kbm);
  }
}

static void clear_kbm()
{
  int i;
  for(i=0;i<keysN;i++) {
    int j;
    for(j=0;j<COLN;j++) {
      GtkWidget *lab = keys[i][j].lab;
      if (lab)
        gtk_label_set_text(GTK_LABEL(lab), NULL);

      if (keys[i][j].laben)
        gtk_label_set_text(GTK_LABEL(keys[i][j].laben), _(keys[i][j].enkey));
    }
  }
}

static void disp_shift_keys()
{
      int i;
      for(i=127; i > 0; i--) {
        char tt[64];
          KEY *p = get_keys_ent(i);
          if (p && p->shift_key) {
            char *t = (char *)gtk_label_get_text(GTK_LABEL(p->lab));
            if (t && t[0])
              continue;
//            dbg("zzz %c %s\n",i, tt);
            tt[0]=p->shift_key;
            tt[1]=0;
            set_kbm_key(i, tt);
          }
      }
}


void update_win_kbm()
{
  if (!current_CS || !gwin_kbm)
    return;

  clear_kbm();

  if (current_CS->im_state != HIME_STATE_CHINESE) {
    if (current_CS->im_state == HIME_STATE_DISABLED) {
      int i;
      for(i=0;i<keysN;i++) {
        int j;
        for(j=0;j<COLN;j++) {
          char kstr[2];
          kstr[1]=0;
          kstr[0] = keys[i][j].shift_key;

          if (keys[i][j].laben) {
            if (kstr[0])
              gtk_label_set_text(GTK_LABEL(keys[i][j].laben), kstr);
            set_label_font_size(keys[i][j].laben, hime_font_size_win_kbm_en);
          }

          if (keys[i][j].lab) {
            if (kstr[0])
              gtk_label_set_text(GTK_LABEL(keys[i][j].lab), _(keys[i][j].enkey));
            set_label_font_size(keys[i][j].lab, hime_font_size_win_kbm_en);
          }
        }
      }
   }
   goto ret;
 }

  int i;
  switch (current_method_type()) {
    case method_type_PHO:
    case method_type_TSIN:
      for(i=0; i < 128; i++) {
        int j;
        char tt[64];
        int ttN=0;

        for(j=0;j<3; j++) {
          int num = phkbm.phokbm[i][j].num;
          int typ = phkbm.phokbm[i][j].typ;
          if (!num)
            continue;
          ttN+= utf8cpy(&tt[ttN], &pho_chars[typ][num * 3]);
        }

        if (!ttN)
         continue;
        set_kbm_key(i, tt);
      }

      disp_shift_keys();

      break;
    case method_type_MODULE:
      break;
    default:
      if (!cur_inmd || !cur_inmd->DefChars)
        return;

      int loop;
      for(loop=0;loop<2;loop++)
      for(i=127; i > 0; i--) {
        char tt[64];
        char k=cur_inmd->keymap[i];
        if (!k)
          continue;

        char *keyname = &cur_inmd->keyname[k * CH_SZ];
        if (!keyname[0])
          continue;

        if (loop==0 && !(keyname[0]&0x80))
          continue;

        if (loop==1) {
          KEY *p = get_keys_ent(i);
          char *t = (char *)gtk_label_get_text(GTK_LABEL(p->lab));
          if (t && t[0]) {
            continue;
          }
        }


        tt[0]=0;
        if (keyname[0] & 128)
          utf8cpy(tt, keyname);
        else {
          tt[1]=0;
          memcpy(tt, keyname, 2);
          tt[2]=0;
        }

//        dbg("%c '%s'\n", i, tt);
        set_kbm_key(i, tt);
      }

      disp_shift_keys();

      break;
  }

ret:
  move_win_kbm();
}


void hide_win_kbm()
{
  if (!gwin_kbm)
    return;
  clear_kbm_timeout_handle();
  win_kbm_on = FALSE;
#if TRAY_ENABLED
  update_item_active_all();
#endif
  gtk_widget_hide(gwin_kbm);
}

extern gboolean old_capslock_on;

void win_kbm_disp_caplock()
{
  KEY *p = get_keys_ent(XK_Caps_Lock);

  if (old_capslock_on) {
 //   dbg("lock...\n");
    mod_fg_all(p->laben, &red);
  } else {
    mod_fg_all(p->laben, NULL);
  }
}
