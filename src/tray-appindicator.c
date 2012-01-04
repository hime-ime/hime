/* Copyright (C) 2011-2012 tytsim <https://github.com/tytsim>
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
#include "gtab.h"
#include "gst.h"
#include <libappindicator/app-indicator.h>
#include "mitem.h"

// NOTE: win-kbm.c Provide GEO information 無解
// NOTE: 左右鍵無解

extern void destroy_other_tray();

AppIndicator *tray_appindicator = NULL;
void init_tray_appindicator();
GtkWidget *create_tray_menu(MITEM *mitems);

void exec_hime_setup_(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_trad_sim_toggle();
void cb_trad_sim_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_sim2trad(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void restart_hime(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_tog_phospeak(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void kbm_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat);
extern int win_kbm_on;

void cb_inmd_menu(GtkCheckMenuItem *checkmenuitem, gpointer dat);

extern int win_kbm_on, gb_output;

void get_icon_path(char *iconame, char fname[]);

void toggle_im_enabled();
void cb_toggle_im_enabled(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_toggle_im_enabled(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  toggle_im_enabled();
}


static MITEM mitems[] = {
  {N_("開關輸入法"), NULL, cb_toggle_im_enabled, NULL},
  {N_("設定"), GTK_STOCK_PREFERENCES, exec_hime_setup_, NULL},
  {N_("重新執行hime"), GTK_STOCK_QUIT, restart_hime, NULL},
  {N_("念出發音"), NULL, cb_tog_phospeak, &phonetic_speak},
  {N_("外部繁轉簡工具"), NULL, cb_trad2sim, NULL},
  {N_("外部簡轉繁工具"), NULL, cb_sim2trad, NULL},
  {N_("選擇輸入法"), NULL, cb_inmd_menu, NULL},
  {N_("小鍵盤"), NULL, kbm_toggle_, &win_kbm_on},
  {N_("輸出成簡體"), NULL, cb_trad_sim_toggle_, &gb_output},
  {NULL, NULL, NULL, NULL}
};
#define HIME_TRAY_ICONAME "hime-tray"
#ifndef HIME_TRAY_PNG
#define HIME_TRAY_PNG "hime-tray.png"
#endif
static void tray_appindicator_load_icon(char iconame[], char fallback[])
{
  char fname[512];
  get_icon_path("", fname);
 
  char fx[128];
  char gx[128];
  char dname[512];
  get_icon_path(iconame, dname);
  sprintf(fx, "%s.png", dname);
  sprintf(gx, HIME_ICON_DIR"/%s.png", iconame);
  if (access(fx, F_OK) == 0) {
    app_indicator_set_icon_theme_path(tray_appindicator, fname);
  } else if (access(gx, F_OK) == 0) {
    app_indicator_set_icon_theme_path(tray_appindicator, HIME_ICON_DIR);
  } else {
    strcpy(iconame, fallback);
    app_indicator_set_icon_theme_path(tray_appindicator, HIME_ICON_DIR);
    if (fallback != HIME_TRAY_ICONAME)
      tray_appindicator_load_icon(iconame, HIME_TRAY_ICONAME);    
  }
}

static void tray_appindicator_update_icon()
{
  char iconame[128];
  if (!current_CS || current_CS->im_state == HIME_STATE_DISABLED||current_CS->im_state == HIME_STATE_ENG_FULL) {
    strcpy(iconame, HIME_TRAY_ICONAME);
  } else {
    strcpy(iconame, inmd[current_CS->in_method].icon);
    iconame[strlen(iconame)-4] = 0;
  }

  if (current_CS && current_CS->im_state == HIME_STATE_CHINESE && !tsin_pho_mode()) {
    if ((current_method_type()==method_type_TSIN || current_method_type()==method_type_MODULE)) {
      char s[128];
      strcpy(s, "en-");
      strcat(s, iconame);
      strcpy(iconame, s);
    } else {
      strcpy(iconame, "en-tsin");
    }
    tray_appindicator_load_icon(iconame, "en-tsin");
  } else {
    tray_appindicator_load_icon(iconame, HIME_TRAY_ICONAME);
  }

  app_indicator_set_icon_full(tray_appindicator, iconame, "");
}

char *tray_appindicator_label_create()
{
  static char st_str[128]="",st_gb[32]="/簡", st_half[32]="半", st_full[32]="全";
  strcpy(st_str, "");
  if (current_CS && (current_CS->im_state == HIME_STATE_ENG_FULL || (current_CS->im_state != HIME_STATE_DISABLED && current_CS->b_half_full_char) || (current_method_type()==method_type_TSIN && tss.tsin_half_full)))
    strcat(st_str, st_full);
  else
    strcat(st_str, st_half);
  if (gb_output)
    strcat(st_str, st_gb);
  return st_str;
}

void load_tray_appindicator()
{
  if (!hime_status_tray)
    return;
  if (!is_exist_tray_appindicator()) {
    init_tray_appindicator();
    return;
  }
  
  app_indicator_set_label(tray_appindicator, tray_appindicator_label_create(), "　　");
  tray_appindicator_update_icon();
}

static void cb_activate(GtkStatusIcon *status_icon, gpointer user_data)
{
  toggle_im_enabled();
}

gboolean tray_appindicator_create(gpointer data)
{
  if (is_exist_tray_appindicator())
    return FALSE;
  destroy_other_tray();
  if (IS_APP_INDICATOR(tray_appindicator) && tray_appindicator) {
    if (app_indicator_get_status (tray_appindicator) == APP_INDICATOR_STATUS_PASSIVE) {
       app_indicator_set_status (tray_appindicator, APP_INDICATOR_STATUS_ACTIVE);
       load_tray_appindicator();
    }
    return FALSE;
  }
  GtkWidget *menu = NULL;
  if (access(HIME_ICON_DIR"/"HIME_TRAY_PNG, F_OK) != 0)
    return FALSE;
  tray_appindicator = app_indicator_new_with_path ("hime", HIME_TRAY_ICONAME, APP_INDICATOR_CATEGORY_APPLICATION_STATUS, HIME_ICON_DIR);
  app_indicator_set_status (tray_appindicator, APP_INDICATOR_STATUS_ACTIVE);
  menu = create_tray_menu(mitems);
// Temporarily comment the following line:
//    app_indicator_set_secondary_activate_target(tray_appindicator, mitems[0].item);
// TODO: implement secondat_activate signal back for older version of libappindicator later
  app_indicator_set_menu (tray_appindicator, GTK_MENU (menu));

  load_tray_appindicator();
  return TRUE;
}


void destroy_tray_appindicator()
{
// Workaround: tytsim: I haven't find the way to destroy appindicator, hide it instead temporarily
  if (tray_appindicator != NULL)
    app_indicator_set_status(tray_appindicator, APP_INDICATOR_STATUS_PASSIVE);
}

gboolean is_exist_tray_appindicator()
{
  return tray_appindicator != NULL && app_indicator_get_status (tray_appindicator) == APP_INDICATOR_STATUS_ACTIVE;
}

void init_tray_appindicator()
{
  g_timeout_add(200, tray_appindicator_create, NULL);
} 
