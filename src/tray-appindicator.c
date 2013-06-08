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

// TODO: win-kbm.c positioning

extern void destroy_other_tray();
gboolean is_exist_tray_appindicator();

AppIndicator *tray_appindicator = NULL;
void init_tray_appindicator();
GtkWidget *create_tray_menu(MITEM *mitems);

void exec_hime_setup_(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_trad_sim_toggle();
void cb_trad_sim_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_sim2trad(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void quit_hime(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_tog_phospeak(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void kbm_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_inmd_menu(GtkCheckMenuItem *checkmenuitem, gpointer dat);

extern gboolean win_kbm_on, gb_output;

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
  {N_("結束hime"), GTK_STOCK_QUIT, quit_hime, NULL},
  {N_("念出發音"), NULL, cb_tog_phospeak, &phonetic_speak},
  {N_("繁轉簡工具"), NULL, cb_trad2sim, NULL},
  {N_("簡轉繁工具"), NULL, cb_sim2trad, NULL},
  {N_("選擇輸入法"), NULL, cb_inmd_menu, NULL},
  {N_("小鍵盤"), NULL, kbm_toggle_, &hime_show_win_kbm},
  {N_("輸出成簡體"), NULL, cb_trad_sim_toggle_, &gb_output},
  {NULL, NULL, NULL, NULL}
};

void update_item_active(MITEM *mitems);
void update_item_active_appindicator()
{
  update_item_active(mitems);
}

#define HIME_TRAY_PNG "hime-tray.png"

static char iconfile[64], icondir[256], iconame[64];
static gboolean tray_appindicator_load_icon(char fallback[], char iconfile[], char iconame[], char icondir[])
{
  char iconpath[256];
  gboolean icon_readable;
  get_icon_path(iconfile, iconpath);
  icon_readable = !access(iconpath, R_OK);
  if (icon_readable) { // iconpath exists
// TODO: check iconpath.endWith(iconfile);
    strcpy(icondir, iconpath);
    icondir[strlen(icondir)-strlen(iconfile)] = 0;
// TODO: check file extension before cut 4 bytes
    strcpy(iconame, iconfile);
    iconame[strlen(iconame)-4] = 0;
    return icon_readable;
  } else if (strcmp(fallback, HIME_TRAY_PNG)) { // iconpath does not exist, then fallback
    strcpy(iconfile, fallback);
    return tray_appindicator_load_icon(HIME_TRAY_PNG, iconfile, iconame, icondir);
  } else {
    return FALSE;
  }
}

extern gboolean tsin_pho_mode();

static void tray_appindicator_update_icon()
{
  if (!current_CS || current_CS->im_state == HIME_STATE_DISABLED||current_CS->im_state == HIME_STATE_ENG_FULL) {
    strcpy(iconfile, HIME_TRAY_PNG);
  } else {
    strcpy(iconfile, inmd[current_CS->in_method].icon);
  }

  if (current_CS && current_CS->im_state == HIME_STATE_CHINESE && !tsin_pho_mode()) {
    char s[64];
    strcpy(s, "en-");
    strcat(s, iconfile);
    strcpy(iconfile, s);
    if (!tray_appindicator_load_icon("en-tsin.png", iconfile, iconame, icondir))
      return;
  } else {
    if (!tray_appindicator_load_icon(HIME_TRAY_PNG, iconfile, iconame, icondir))
      return;
  }
  
  app_indicator_set_icon_theme_path(tray_appindicator, icondir);
  app_indicator_set_icon_full(tray_appindicator, iconame, "");
}

static char st_gb[]=N_("/簡"), st_half[]=N_("半"), st_full[]=N_("全"), st_str[32];
extern int current_shape_mode();
static char * tray_appindicator_label_create()
{
  strcpy(st_str, "");
  if (current_shape_mode())
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

  app_indicator_set_label(tray_appindicator, tray_appindicator_label_create(), "　/　");
  tray_appindicator_update_icon();
}

gboolean tray_appindicator_create(gpointer data)
{
  if (is_exist_tray_appindicator())
    return FALSE;

  if (tray_appindicator) {
    if (app_indicator_get_status (tray_appindicator) != APP_INDICATOR_STATUS_ACTIVE) {
      app_indicator_set_status (tray_appindicator, APP_INDICATOR_STATUS_ACTIVE);
      destroy_other_tray();
    }
  } else {
    destroy_other_tray();

    if (!tray_appindicator_load_icon(HIME_TRAY_PNG, HIME_TRAY_PNG, iconame, icondir))
      return FALSE;

    tray_appindicator = app_indicator_new_with_path ("hime", iconame, APP_INDICATOR_CATEGORY_APPLICATION_STATUS, icondir);
    if(tray_appindicator == NULL)
      return TRUE;

    app_indicator_set_status (tray_appindicator, APP_INDICATOR_STATUS_ACTIVE);
    GtkWidget *menu = NULL;
    menu = create_tray_menu(mitems);
    app_indicator_set_secondary_activate_target(tray_appindicator, mitems[0].item);
    app_indicator_set_menu (tray_appindicator, GTK_MENU (menu));
  }

  load_tray_appindicator();
  return FALSE;
}


void destroy_tray_appindicator()
{
// Workaround: tytsim: I haven't find the way to destroy appindicator, hide it instead temporarily
  if (tray_appindicator != NULL)
    app_indicator_set_status(tray_appindicator, APP_INDICATOR_STATUS_PASSIVE);
}

gboolean is_exist_tray_appindicator()
{
  return tray_appindicator != NULL && app_indicator_get_status (tray_appindicator) != APP_INDICATOR_STATUS_PASSIVE;
}

void init_tray_appindicator()
{
  g_timeout_add(200, tray_appindicator_create, NULL);
} 
