/* Copyright (C) 2011 cwlin <https://github.com/cwlin>
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

#include "chewing.h"

// TODO:
//     the hbox/label could be moved to local func
static GtkWidget *hime_chewing_window = NULL;
static GtkWidget *vbox_top = NULL;
static GtkWidget *hbox_cancel_ok = NULL;
static GtkWidget *button_cancel = NULL;
static GtkWidget *button_ok = NULL;

static GtkWidget *g_pHBoxCandPerPage = NULL;
static GtkWidget *g_pLabelCandPerPage = NULL;
static GtkAdjustment *g_pGtkAdj = NULL;
static GtkWidget *g_pSpinButtonCandPerPage = NULL;

static GtkWidget *g_pHBoxSpaceAsSelection = NULL;
static GtkWidget *g_pLabelSpaceAsSelection = NULL;
static GtkWidget *g_pCheckButtonSpaceAsSelection = NULL;

static GtkWidget *g_pHBoxEscCleanAllBuf = NULL;
static GtkWidget *g_pLabelEscCleanAllBuf = NULL;
static GtkWidget *g_pCheckButtonEscCleanAllBuf = NULL;

static GtkWidget *g_pHBoxAutoShiftCur = NULL;
static GtkWidget *g_pLabelAutoShiftCur = NULL;
static GtkWidget *g_pCheckButtonAutoShiftCur = NULL;

static GtkWidget *g_pHBoxAddPhraseForward = NULL;
static GtkWidget *g_pLabelAddPhraseForward = NULL;
static GtkWidget *g_pCheckButtonAddPhraseForward = NULL;

static ChewingConfigData g_chewingConfig;

static gboolean
cb_close_window (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    chewing_config_close ();

    gtk_widget_destroy (hime_chewing_window);
    hime_chewing_window = NULL;
    memset (&g_chewingConfig, 0x00, sizeof (g_chewingConfig));

    return TRUE;
}

static gboolean
cb_update_setting (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    int nVal[] = 
    {
        gtk_spin_button_get_value (GTK_SPIN_BUTTON (g_pSpinButtonCandPerPage)),
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_pCheckButtonSpaceAsSelection)),
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_pCheckButtonEscCleanAllBuf)),
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_pCheckButtonAutoShiftCur)),
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_pCheckButtonAddPhraseForward)) 
    };

    if (!chewing_config_save (nVal))
    {
        chewing_config_close ();
        return FALSE;
    }

    chewing_config_close ();

    gtk_widget_destroy (hime_chewing_window);
    hime_chewing_window = NULL;
    return TRUE;
}

void module_setup_window_create ()
{
    gboolean bWriteMode = TRUE;

    chewing_config_open (bWriteMode);
    
    chewing_config_load (&g_chewingConfig);

    if (hime_chewing_window)
    {
        gtk_window_present (GTK_WINDOW (hime_chewing_window));
        return;
    }

    hime_chewing_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    /* main setup win setting */
    gtk_window_set_position (GTK_WINDOW (hime_chewing_window),
                             GTK_WIN_POS_MOUSE);
    gtk_window_set_has_resize_grip (GTK_WINDOW (hime_chewing_window), FALSE);

    g_signal_connect (G_OBJECT (hime_chewing_window), "delete_event",
                      G_CALLBACK (cb_close_window),
                      NULL);

    gtk_window_set_title (GTK_WINDOW (hime_chewing_window),
                          _(_L("hime 新酷音設定")));
    gtk_container_set_border_width (GTK_CONTAINER (hime_chewing_window), 1);

    vbox_top = gtk_vbox_new (FALSE, 3);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_top), GTK_ORIENTATION_VERTICAL);
    gtk_container_add (GTK_CONTAINER (hime_chewing_window), vbox_top);

    // cand per page
    g_pHBoxCandPerPage = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), g_pHBoxCandPerPage, TRUE, TRUE, 1);
    g_pLabelCandPerPage = gtk_label_new (_(_L("每頁候選字數")));
    gtk_box_pack_start (GTK_BOX (g_pHBoxCandPerPage), g_pLabelCandPerPage, TRUE, TRUE, 0);
    g_pGtkAdj = (GtkAdjustment *)gtk_adjustment_new (g_chewingConfig.candPerPage, 1, 10, 1.0, 1.0, 0.0);
    g_pSpinButtonCandPerPage = gtk_spin_button_new (g_pGtkAdj, 0, 0);
    gtk_box_pack_start (GTK_BOX (g_pHBoxCandPerPage), g_pSpinButtonCandPerPage, FALSE, FALSE, 0);

    // space as selection
    g_pHBoxSpaceAsSelection = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), g_pHBoxSpaceAsSelection, TRUE, TRUE, 1);
    g_pCheckButtonSpaceAsSelection = gtk_check_button_new_with_label(_(_L("空白鍵選字")));
    gtk_box_pack_start (GTK_BOX (g_pHBoxSpaceAsSelection), g_pCheckButtonSpaceAsSelection, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (g_pCheckButtonSpaceAsSelection), g_chewingConfig.bSpaceAsSelection);

    // esc clean buf
    g_pHBoxEscCleanAllBuf = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), g_pHBoxEscCleanAllBuf, TRUE, TRUE, 1);
    g_pCheckButtonEscCleanAllBuf = gtk_check_button_new_with_label (_(_L("ESC 鍵清空緩衝區")));
    gtk_box_pack_start (GTK_BOX (g_pHBoxEscCleanAllBuf), g_pCheckButtonEscCleanAllBuf, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (g_pCheckButtonEscCleanAllBuf), g_chewingConfig.bEscCleanAllBuf);

    // auto shift cursor
    g_pHBoxAutoShiftCur = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), g_pHBoxAutoShiftCur, TRUE, TRUE, 1);
    g_pCheckButtonAutoShiftCur = gtk_check_button_new_with_label (_(_L("選字完畢自動跳字")));
    gtk_box_pack_start (GTK_BOX (g_pHBoxAutoShiftCur), g_pCheckButtonAutoShiftCur, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (g_pCheckButtonAutoShiftCur), g_chewingConfig.bAutoShiftCur);

    // add phrase forward
    g_pHBoxAddPhraseForward = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), g_pHBoxAddPhraseForward, TRUE, TRUE, 1);
    g_pCheckButtonAddPhraseForward = gtk_check_button_new_with_label (_(_L("向後加詞")));
    gtk_box_pack_start (GTK_BOX (g_pHBoxAddPhraseForward), g_pCheckButtonAddPhraseForward, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (g_pCheckButtonAddPhraseForward), g_chewingConfig.bAddPhraseForward);

    // cancel & ok buttons
    hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_cancel_ok , FALSE, FALSE, 5);

    button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    gboolean button_order;
    g_object_get(gtk_settings_get_default(), "gtk-alternative-button-order", &button_order, NULL);
    if (button_order)
      gtk_box_pack_end (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);
    else
      gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);
    button_ok = gtk_button_new_from_stock (GTK_STOCK_OK);
#if !GTK_CHECK_VERSION(2,91,2)
    if (button_order)
      gtk_box_pack_end (GTK_BOX (hbox_cancel_ok), button_ok, TRUE, TRUE, 5);
    else
      gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_ok, TRUE, TRUE, 5);
#else
    if (button_order)
      gtk_grid_attach_next_to (GTK_BOX (hbox_cancel_ok), button_ok, button_cancel, GTK_POS_LEFT, 1, 1);
    else
      gtk_grid_attach_next_to (GTK_BOX (hbox_cancel_ok), button_ok, button_cancel, GTK_POS_RIGHT, 1, 1);
#endif

    g_signal_connect (G_OBJECT (button_cancel), "clicked",
                      G_CALLBACK (cb_close_window),
                      G_OBJECT (hime_chewing_window));

    g_signal_connect (G_OBJECT (button_ok), "clicked",
                      G_CALLBACK (cb_update_setting),
                      G_OBJECT (hime_chewing_window));

    gtk_widget_show_all (hime_chewing_window);
}
