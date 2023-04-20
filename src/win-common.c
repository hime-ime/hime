/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include "win-common.h"

char *get_full_str () {
    if (!chinese_mode ()) {
        if (hime_use_custom_theme)
            return eng_color_half_str;
        else
            return _ (eng_half_str);
    }

    if (current_CS->b_im_enabled) {
        if (current_fullshape_mode ()) {
            if (hime_use_custom_theme)
                return cht_color_full_str;
            else
                return _ (cht_full_str);
        }
    } else if (current_fullshape_mode ()) {
        if (hime_use_custom_theme)
            return eng_color_full_str;
        else
            return _ (eng_full_str);
    }
    return ("");
}

void get_win_geom (GtkWidget *win) {
    if (!win)
        return;
    gtk_window_get_position (GTK_WINDOW (win), &win_x, &win_y);
    get_win_size (win, &win_xl, &win_yl);
}

void get_win_size (GtkWidget *win, int *width, int *height) {
    GtkRequisition sz;
    sz.width = sz.height = 0;
    gtk_widget_get_preferred_size (GTK_WIDGET (win), NULL, &sz);
    *width = sz.width;
    *height = sz.height;
}
