/* Copyright (C) 2011-2012 cwlin <https://github.com/cwlin>
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

// hime-chewing funcs
static gboolean select_idx (int c);
static void prev_page (void);
static void next_page (void);
static gboolean chewing_initialize (void);
static gboolean is_empty (void);
static gboolean hime_label_show (char *pszPho, int nPos);
static gboolean hime_label_clear (int nCount);
static gboolean hime_label_cand_show (char *pszWord, int nCount);
static gboolean gtk_pango_font_pixel_size_get (int *pnFontWidth, int *pnFontHeight);

static gboolean hime_key_filter (int *pnKeyVal);
static gboolean hime_zuin_label_show (void);
static gboolean hime_buffer_label_show (void);
static gboolean hime_buffer_commit (void);

static void hime_chewing_cb_register (void);
static void hime_chewing_handler_default (ChewingContext *pCtx);

static int hime_chewing_wrapper_bs (ChewingContext *pCtx);
static int hime_chewing_wrapper_enter (ChewingContext *pCtx);
static int hime_chewing_wrapper_home (ChewingContext *pCtx);
static int hime_chewing_wrapper_left (ChewingContext *pCtx);
static int hime_chewing_wrapper_up (ChewingContext *pCtx);
static int hime_chewing_wrapper_right (ChewingContext *pCtx);
static int hime_chewing_wrapper_down (ChewingContext *pCtx);
static int hime_chewing_wrapper_pageup (ChewingContext *pCtx);
static int hime_chewing_wrapper_pagedown (ChewingContext *pCtx);
static int hime_chewing_wrapper_end (ChewingContext *pCtx);
static int hime_chewing_wrapper_del (ChewingContext *pCtx);

static HIME_module_main_functions g_himeModMainFuncs;
static GtkWidget *g_pWinChewing      = NULL;
static ChewingContext *g_pChewingCtx = NULL;
static GtkWidget *g_pEvBoxChewing    = NULL;
static GtkWidget *g_pHBoxChewing     = NULL;
static SEG *g_pSeg = NULL;
static int g_nCurrentCursorPos = 0;

static intptr_t (*g_pKeyHandler[HIME_CHEWING_KEY_MAX]) (ChewingContext *pCtx);

// FIXME: impl
static gboolean
select_idx (int c)
{
    return TRUE;
}

// FIXME: impl
static void
prev_page (void)
{
}

// FIXME: impl
static void
next_page (void)
{
}

static gboolean
hime_label_show (char *pszPho, int nPos)
{
    char szTmp[128];

    if (!pszPho)
        return FALSE;

    memset (szTmp, 0x00, 128);

    if (*g_himeModMainFuncs.mf_hime_win_color_use)
      sprintf (szTmp, "<span background=\"%s\" foreground=\"white\">%s</span>",
             *g_himeModMainFuncs.mf_tsin_cursor_color,
             pszPho);
    else
      sprintf (szTmp, "<span background=\""TSIN_CURSOR_COLOR_DEFAULT"\">%s</span>",
             pszPho);

    gtk_label_set_markup (GTK_LABEL (g_pSeg[nPos].label),
                          nPos != g_nCurrentCursorPos ? pszPho : szTmp);

    return TRUE;
}

static gboolean
hime_label_clear (int nCount)
{
    while (nCount--)
        gtk_label_set_text (GTK_LABEL (g_pSeg[nCount].label), NULL);

    return TRUE;
}

static gboolean
gtk_pango_font_pixel_size_get (int *pnFontWidth, int *pnFontHeight)
{
    PangoLayout *pPangoLayout;
    PangoContext *pPangoContext;
    PangoFontDescription *pPangoFontDesc;

    pPangoLayout = gtk_widget_create_pango_layout (g_pWinChewing, "中");
    //pPangoLayout = gtk_widget_create_pango_layout (
    //                   g_pWinChewing,
    //                   (char *)gtk_label_get_text (GTK_LABEL (g_pSeg[g_nCurrentCursorPos].label)));
    pPangoContext = gtk_widget_get_pango_context (g_pWinChewing);
    pPangoFontDesc = pango_context_get_font_description (pPangoContext);

    pango_layout_set_font_description (pPangoLayout, pPangoFontDesc);
    pango_layout_get_pixel_size (pPangoLayout, pnFontWidth, pnFontHeight);

    g_object_unref (pPangoLayout);
    return TRUE;
}

// FIXME: the pos of g_pSeg[].label is not correct
static gboolean
hime_label_cand_show (char *pszWord, int nIdx)
{
    int nX, nY;
    int nFontWidth, nFontHeight;

    g_himeModMainFuncs.mf_set_sele_text (chewing_cand_TotalChoice (g_pChewingCtx),
                                         nIdx,
                                         pszWord,
                                         -1);

    // find the position of the cand win
    g_himeModMainFuncs.mf_get_widget_xy (g_pWinChewing,
                                         g_pSeg[g_nCurrentCursorPos].label,
                                         &nX, &nY);

    gtk_pango_font_pixel_size_get (&nFontWidth, &nFontHeight);
    nX += g_nCurrentCursorPos * nFontWidth;

    nY = g_himeModMainFuncs.mf_hime_edit_display_ap_only () ?
         *g_himeModMainFuncs.mf_win_y :
         *g_himeModMainFuncs.mf_win_y + *g_himeModMainFuncs.mf_win_yl;

    g_himeModMainFuncs.mf_disp_selections (nX, nY);

    return TRUE;
}

static gboolean
chewing_initialize (void)
{
    char *pszChewingHashDir;
    char *pszHome;
    gboolean bWriteMode = FALSE;
    ChewingConfigData dummyConfig;

    pszHome = getenv ("HOME");
    if (!pszHome)
        pszHome = "";

    pszChewingHashDir = malloc (strlen (pszHome) + strlen ("/.chewing/") + 1);
    memset (pszChewingHashDir, 0x00, strlen (pszHome) + strlen ("/.chewing/") + 1);
    sprintf (pszChewingHashDir, "%s/.chewing", pszHome);

    if (chewing_Init (CHEWING_DATADIR, pszChewingHashDir) != 0)
    {
        free (pszChewingHashDir);
        return FALSE;
    }
    free (pszChewingHashDir);
    pszHome = NULL;

    g_pChewingCtx = chewing_new ();
    if (!g_pChewingCtx)
        return FALSE;

    memset (&dummyConfig, 0x00, sizeof (ChewingConfigData));

    chewing_config_open (bWriteMode);

    chewing_config_load (&dummyConfig);

    chewing_config_set (g_pChewingCtx);

    chewing_config_close ();

    hime_chewing_cb_register ();

    return TRUE;
}

static gboolean
is_empty (void)
{
    if (!g_pChewingCtx)
        return FALSE;
    int  nZuinLen = 0;
    free(chewing_zuin_String (g_pChewingCtx, &nZuinLen));
    return !chewing_buffer_Len (g_pChewingCtx) && !nZuinLen;
}

static gboolean 
hime_key_filter (int *pnKeyVal)
{
    if ((*pnKeyVal) > HIME_CHEWING_DEFAULT_KEY_MIN && 
        (*pnKeyVal) < HIME_CHEWING_DEFAULT_KEY_MAX)
        chewing_handle_Default (g_pChewingCtx, (*pnKeyVal));
    else if ((*pnKeyVal) >= XK_KP_0 && (*pnKeyVal) <= XK_KP_9)
        chewing_handle_Numlock (g_pChewingCtx, (*pnKeyVal) - XK_KP_0 + '0');
    else if ((*pnKeyVal) < HIME_CHEWING_KEY_MAX && 
             (*pnKeyVal) >= HIME_CHEWING_KEY_MIN)
        if ((int)(g_pKeyHandler[(*pnKeyVal)] (g_pChewingCtx)) == -1)
            return FALSE;

    g_nCurrentCursorPos = chewing_cursor_Current (g_pChewingCtx);

    if (g_nCurrentCursorPos < 0 || g_nCurrentCursorPos > MAX_SEG_NUM)
        return FALSE;

    return TRUE;
}

static gboolean
hime_zuin_label_show (void)
{
    char *pszTmp  = NULL;
    char *pszWord = NULL;
    int  nZuinLen = 0, nIdx = 0, nPhoIdx = 0;

    pszTmp = chewing_zuin_String (g_pChewingCtx, &nZuinLen);
    pszWord = (char *) realloc (pszWord, 4);

    if (!pszWord)
        return FALSE;

    memset (pszWord, 0x00, 4);

    if (pszTmp)
    {
        for (nIdx = 0; nIdx < nZuinLen; nIdx++)
        {
            memcpy (pszWord, pszTmp + nIdx * 3, 3);
            for (nPhoIdx = 0; nPhoIdx < 3; nPhoIdx++)
                if (strstr (g_himeModMainFuncs.mf_pho_chars[nPhoIdx], pszWord) != NULL)
                    hime_label_show (pszWord, nPhoIdx + chewing_buffer_Len (g_pChewingCtx) + 1);
        }

        free (pszTmp);
    }

    free (pszWord);

    return TRUE;
}

static gboolean 
hime_buffer_label_show (void)
{
    char *pszTmp         = NULL;
    char *pszWord        = NULL;
    char *pszChewingCand = NULL;
    char *pHead          = NULL;
    int  nIdx            = 0;
    int  nPos            = 0;
    int  nWordSize       = 0; 

    pszWord = (char *) realloc (pszWord, 8);

    if (!pszWord)
        return FALSE;

    memset (pszWord, 0x00, 8);

    // check if the composing is valid or not
    if (chewing_buffer_Check (g_pChewingCtx))
    {
        g_himeModMainFuncs.mf_hide_selections_win ();
        pszTmp = chewing_buffer_String (g_pChewingCtx);

        // init cand_no
        chewing_cand_Enumerate (g_pChewingCtx);

        g_himeModMainFuncs.mf_clear_sele ();

        if (chewing_cand_TotalChoice (g_pChewingCtx))
        {
            while (chewing_cand_hasNext (g_pChewingCtx))
            {
                pszChewingCand = chewing_cand_String (g_pChewingCtx);

                if (nIdx > chewing_get_candPerPage (g_pChewingCtx) - 1)
                    break;
                hime_label_cand_show (pszChewingCand, nIdx++);
                free (pszChewingCand);
            }
        }

        for (nPos = 0, pHead = pszTmp, nIdx = 0; nPos < strlen (pszTmp); nPos += nWordSize, pHead += nWordSize)
        {
            if (!((*pHead) & 0x80))               // 1 byte utf-8 data
                nWordSize = 1;
            else if (((*pHead) & 0xf0) == 0xc0)   // 2 bytes utf-8 data
                nWordSize = 2;
            else if (((*pHead) & 0xf0) == 0xe0)   // 3 bytes utf-8 data
                nWordSize = 3;
            else if (((*pHead) & 0xf0) == 0xf0)   // 4 bytes utf-8 data
                nWordSize = 4;

            memset (pszWord, 0x00, 8);
            memcpy (pszWord, pHead, nWordSize);
            hime_label_show (pszWord, nIdx++);
        }

        // if chewing_buffer_Check is not zero, 
        // it means that the chewing_buffer_String must have val, 
        // so we could free the ptr here
        free (pszTmp);
    }

    free (pszWord);

    return TRUE;
}

static gboolean 
hime_buffer_commit (void) 
{
    char *pszTmp = NULL;

    if (chewing_commit_Check (g_pChewingCtx))
    {
        pszTmp = chewing_commit_String (g_pChewingCtx);
        g_himeModMainFuncs.mf_send_text (pszTmp);

        // FIXME: workaround for repeated commit
        chewing_handle_Esc (g_pChewingCtx);

        // if chewing_commit_Check is not zero, 
        // it means that the chewing_commit_String must have val, 
        // so we could free the ptr here
        free (pszTmp);
    }

    if (*g_himeModMainFuncs.mf_hime_pop_up_win && is_empty())
        module_hide_win();

    return TRUE;
}

static void
hime_chewing_handler_default (ChewingContext *pCtx)
{
    return ((void)NULL);
}

static int 
hime_chewing_wrapper_bs (ChewingContext *pCtx)
{
//  If zuin is present, force libchewing handles Backspace for removing last zuin
    int  nZuinLen = 0;
    free(chewing_zuin_String (g_pChewingCtx, &nZuinLen));
    if (nZuinLen)
        return chewing_handle_Backspace (g_pChewingCtx);
    HIME_CHEWING_WRAPPER_FUNC (chewing_handle_Backspace);
}

static int 
hime_chewing_wrapper_enter (ChewingContext *pCtx)
{
    HIME_CHEWING_WRAPPER_FUNC (chewing_handle_Enter);
}

static int 
hime_chewing_wrapper_home (ChewingContext *pCtx)
{
    HIME_CHEWING_WRAPPER_FUNC (chewing_handle_Home);
}

static int 
hime_chewing_wrapper_left (ChewingContext *pCtx)
{
    HIME_CHEWING_WRAPPER_FUNC (chewing_handle_Left);
}

static int 
hime_chewing_wrapper_up (ChewingContext *pCtx)
{
    HIME_CHEWING_WRAPPER_FUNC (chewing_handle_Up);
}

static int 
hime_chewing_wrapper_right (ChewingContext *pCtx)
{
    HIME_CHEWING_WRAPPER_FUNC (chewing_handle_Right);
}

static int 
hime_chewing_wrapper_down (ChewingContext *pCtx)
{
    HIME_CHEWING_WRAPPER_FUNC (chewing_handle_Down);
}

static int 
hime_chewing_wrapper_pageup (ChewingContext *pCtx)
{
    HIME_CHEWING_WRAPPER_FUNC (chewing_handle_PageUp);
}

static int hime_chewing_wrapper_pagedown (ChewingContext *pCtx)
{
    HIME_CHEWING_WRAPPER_FUNC (chewing_handle_PageDown);
}

static int 
hime_chewing_wrapper_end (ChewingContext *pCtx)
{
    HIME_CHEWING_WRAPPER_FUNC (chewing_handle_End);
}

static int 
hime_chewing_wrapper_del (ChewingContext *pCtx)
{
    HIME_CHEWING_WRAPPER_FUNC (chewing_handle_Del);
}

static void 
hime_chewing_cb_register (void)
{
    int nIdx = HIME_CHEWING_KEY_MIN;

    for (; nIdx < HIME_CHEWING_KEY_MAX; nIdx++)
        g_pKeyHandler[nIdx] = (void *)hime_chewing_handler_default;

    g_pKeyHandler[XK_space]     = (void *)chewing_handle_Space;
    g_pKeyHandler[XK_BackSpace] = (void *)hime_chewing_wrapper_bs;
    g_pKeyHandler[XK_Tab]       = (void *)chewing_handle_Tab;
    g_pKeyHandler[XK_Return]    = (void *)hime_chewing_wrapper_enter;
    g_pKeyHandler[XK_Escape]    = (void *)chewing_handle_Esc;
    g_pKeyHandler[XK_Home]      = (void *)hime_chewing_wrapper_home;
    g_pKeyHandler[XK_Left]      = (void *)hime_chewing_wrapper_left;
    g_pKeyHandler[XK_Up]        = (void *)hime_chewing_wrapper_up;
    g_pKeyHandler[XK_Right]     = (void *)hime_chewing_wrapper_right;
    g_pKeyHandler[XK_Down]      = (void *)hime_chewing_wrapper_down;
    g_pKeyHandler[XK_Page_Up]   = (void *)hime_chewing_wrapper_pageup;
    g_pKeyHandler[XK_Page_Down] = (void *)hime_chewing_wrapper_pagedown;
    g_pKeyHandler[XK_End]       = (void *)hime_chewing_wrapper_end;
    g_pKeyHandler[XK_KP_Enter]  = (void *)hime_chewing_wrapper_enter;
    g_pKeyHandler[XK_KP_Left]   = (void *)hime_chewing_wrapper_left;
    g_pKeyHandler[XK_KP_Up]     = (void *)hime_chewing_wrapper_up;
    g_pKeyHandler[XK_KP_Right]  = (void *)hime_chewing_wrapper_right;
    g_pKeyHandler[XK_KP_Down]   = (void *)hime_chewing_wrapper_down;
    g_pKeyHandler[XK_KP_Delete] = (void *)hime_chewing_wrapper_del;
#if 0
    g_pKeyHandler[XK_Shift_L]   = (void *)chewing_handle_ShiftLeft;
    g_pKeyHandler[XK_Shift_R]   = (void *)chewing_handle_ShiftRight;
#endif
    g_pKeyHandler[XK_Delete]    = (void *)hime_chewing_wrapper_del;
}

int
module_init_win (HIME_module_main_functions *pFuncs)
{
    GtkWidget *pErrDialog = NULL;
    int nIdx;
    int nSegSize = 0;

    if (!pFuncs)
        return FALSE;

    g_himeModMainFuncs = *pFuncs;

    g_himeModMainFuncs.mf_set_tsin_pho_mode ();
    g_himeModMainFuncs.mf_set_win1_cb ((cb_selec_by_idx_t)select_idx,
                                        prev_page,
                                        next_page);

    if (g_pWinChewing)
        return TRUE;

    if (!chewing_initialize ())
    {
        pErrDialog = gtk_message_dialog_new (NULL,
                         GTK_DIALOG_MODAL,
                         GTK_MESSAGE_ERROR,
                         GTK_BUTTONS_CLOSE,
                         "chewing init failed");
        gtk_dialog_run (GTK_DIALOG (pErrDialog));
        gtk_widget_destroy (pErrDialog);
        return FALSE;
    }

    g_pWinChewing = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_has_resize_grip (GTK_WINDOW (g_pWinChewing), FALSE);

    gtk_widget_realize (g_pWinChewing);
    g_himeModMainFuncs.mf_set_no_focus (g_pWinChewing);

    g_pEvBoxChewing = gtk_event_box_new ();
    gtk_event_box_set_visible_window (GTK_EVENT_BOX(g_pEvBoxChewing), FALSE);

    if (!g_pEvBoxChewing)
        return FALSE;
    gtk_container_add (GTK_CONTAINER (g_pWinChewing), g_pEvBoxChewing);

    g_pHBoxChewing = gtk_hbox_new (FALSE, 0);
    if (!g_pHBoxChewing)
        return FALSE;
    gtk_container_add (GTK_CONTAINER (g_pEvBoxChewing), g_pHBoxChewing);

    // TODO: not sure if we need the mouse CB or not...
    //        if we add the CB here, then it seems that we have to use gdk
    //        it seems not so important...? do it later
    //g_signal_connect (G_OBJECT (g_pEvBoxChewing), "button-press-event",
    //                  G_CALLBACK (mouse_button_callback), NULL);

    if (!g_pSeg)
    {
        nSegSize = sizeof (SEG) * MAX_SEG_NUM;
        g_pSeg = malloc (nSegSize);
        memset (g_pSeg, 0, nSegSize);
    }

    for (nIdx = 0; nIdx < MAX_SEG_NUM; nIdx++)
    {
        g_pSeg[nIdx].label = gtk_label_new (NULL);
        gtk_widget_show (g_pSeg[nIdx].label);
        gtk_box_pack_start (GTK_BOX (g_pHBoxChewing),
                            g_pSeg[nIdx].label,
                            FALSE,
                            FALSE,
                            0);
    }

    if (!g_himeModMainFuncs.mf_phkbm->selkeyN)
        g_himeModMainFuncs.mf_load_tab_pho_file();

    gtk_widget_show_all (g_pWinChewing);

    g_himeModMainFuncs.mf_init_tsin_selection_win ();

    module_change_font_size ();

    module_hide_win ();

    return TRUE;
}

// FIXME: impl
void
module_get_win_geom (void)
{
    return;
}

// FIXME: chk
int
module_reset (void)
{
    if (!g_pWinChewing)
        return 0;
    return 1;
}

// FIXME: refine and chk
int
module_get_preedit (char *pszStr, HIME_PREEDIT_ATTR himePreeditAttr[],
                    int *pnCursor, int *pCompFlag)
{
    char *pszTmpStr = NULL;
    char *pszZuinStr = NULL;
    int nIdx;
    int nLength;
    int nTotalLen = 0;
    int nAttr = 0;
    int nZuinLen = 0;

    pszStr[0] = 0;
    *pnCursor = 0;
    himePreeditAttr[0].flag = HIME_PREEDIT_ATTR_FLAG_UNDERLINE;
    himePreeditAttr[0].ofs0 = 0;

    if (chewing_buffer_Len (g_pChewingCtx))
        nAttr = 1;

    for (nIdx = 0; nIdx < chewing_buffer_Len (g_pChewingCtx); nIdx++)
    {
        pszTmpStr = (char *)gtk_label_get_text (GTK_LABEL (g_pSeg[nIdx].label));
        nLength = g_himeModMainFuncs.mf_utf8_str_N (pszTmpStr);
        nTotalLen += nLength;

        if (nIdx < chewing_cursor_Current (g_pChewingCtx))
            *pnCursor += nLength;

#if 0
        if (nIdx == chewing_cursor_Current (g_pChewingCtx))
        {
            himePreeditAttr[1].ofs0 = *pnCursor;
            himePreeditAttr[1].ofs1 = *pnCursor + nLength;
            himePreeditAttr[1].flag = HIME_PREEDIT_ATTR_FLAG_REVERSE;
            nAttr++;
        }
#endif

        strcat (pszStr, pszTmpStr);
    }

    if (g_himeModMainFuncs.mf_hime_display_on_the_spot_key()) {
        pszZuinStr = chewing_zuin_String (g_pChewingCtx, &nZuinLen);
	    strcat (pszStr, pszZuinStr);
        free (pszZuinStr);
        nTotalLen += nZuinLen;
    }

    himePreeditAttr[0].ofs1 = nTotalLen;

    pCompFlag = 0;

    return nAttr;
}

gboolean
module_feedkey (int nKeyVal, int nKeyState)
{
    if (!g_pChewingCtx)
        return FALSE;

    if (!g_himeModMainFuncs.mf_tsin_pho_mode ())
        return FALSE;

    hime_label_clear (MAX_SEG_NUM);

    chewing_set_ShapeMode (g_pChewingCtx, g_himeModMainFuncs.mf_current_shape_mode());

    if (nKeyState & (Mod1Mask|Mod4Mask|Mod5Mask|ControlMask))
        return FALSE;

    if (!hime_key_filter (&nKeyVal))
        return FALSE;

    if (!hime_buffer_commit ())
        return FALSE;

    if (!hime_buffer_label_show ())
        return FALSE;

    if (!hime_zuin_label_show ())
        return FALSE;

    module_show_win ();

    return TRUE;
}

// FIXME: impl
int
module_feedkey_release (KeySym xkey, int nKbState)
{
    return 0;
}

void
module_move_win (int nX, int nY)
{
    gtk_window_get_size (GTK_WINDOW (g_pWinChewing),
                         g_himeModMainFuncs.mf_win_xl,
                         g_himeModMainFuncs.mf_win_yl);

    if (nX + *g_himeModMainFuncs.mf_win_xl > *g_himeModMainFuncs.mf_dpy_xl)
        nX = *g_himeModMainFuncs.mf_dpy_xl - *g_himeModMainFuncs.mf_win_xl;
    if (nX < 0)
        nX = 0;

    if (nY + *g_himeModMainFuncs.mf_win_yl > *g_himeModMainFuncs.mf_dpy_yl)
        nY = *g_himeModMainFuncs.mf_dpy_yl - *g_himeModMainFuncs.mf_win_yl;
    if (nY < 0)
        nY = 0;

    gtk_window_move (GTK_WINDOW(g_pWinChewing), nX, nY);

    *g_himeModMainFuncs.mf_win_x = nX;
    *g_himeModMainFuncs.mf_win_y = nY;

    g_himeModMainFuncs.mf_move_win_sym ();
}

void
module_change_font_size (void)
{
    GdkColor colorFG;
    GtkWidget *pLabel;
    int n;

    gdk_color_parse (*g_himeModMainFuncs.mf_hime_win_color_fg, &colorFG);
    g_himeModMainFuncs.mf_change_win_bg (g_pWinChewing);
    g_himeModMainFuncs.mf_change_win_bg (g_pEvBoxChewing);

    for (n = 0; n < MAX_SEG_NUM; n++)
    {
        pLabel = g_pSeg[n].label;
        g_himeModMainFuncs.mf_set_label_font_size (pLabel,
            *g_himeModMainFuncs.mf_hime_font_size);

        if (*g_himeModMainFuncs.mf_hime_win_color_use) {
#if !GTK_CHECK_VERSION(2,91,6)
            gtk_widget_modify_fg (pLabel, GTK_STATE_NORMAL, &colorFG);
#else
            GdkRGBA rgbfg;
            gdk_rgba_parse(&rgbfg, gdk_color_to_string(&colorFG));
            gtk_widget_override_color(pLabel, GTK_STATE_FLAG_NORMAL, &rgbfg);
#endif
        }
    }
}

void
module_show_win (void)
{
    if (g_himeModMainFuncs.mf_hime_edit_display_ap_only ())
        return;

    if (*g_himeModMainFuncs.mf_hime_pop_up_win && is_empty ())
        return;

    gtk_window_resize (GTK_WINDOW (g_pWinChewing),
                       32 * (chewing_buffer_Check (g_pChewingCtx) + 1),
                       12);
    gtk_widget_show (g_pWinChewing);

    g_himeModMainFuncs.mf_show_win_sym ();
}

void
module_hide_win (void)
{
    gtk_widget_hide (g_pWinChewing);
    g_himeModMainFuncs.mf_hide_selections_win ();
    g_himeModMainFuncs.mf_hide_win_sym ();
}

// FIXME: chk
int
module_win_visible (void)
{
    return GTK_WIDGET_VISIBLE (g_pWinChewing);
}

void
module_win_geom (void)
{
    if (!g_pWinChewing)
        return;

    gtk_window_get_position(GTK_WINDOW(g_pWinChewing),
                            g_himeModMainFuncs.mf_win_x,
                            g_himeModMainFuncs.mf_win_y);

    g_himeModMainFuncs.mf_get_win_size(g_pWinChewing,
                                       g_himeModMainFuncs.mf_win_xl,
                                       g_himeModMainFuncs.mf_win_yl);
}

int
module_flush_input (void)
{
    char *pszTmp;

    if (chewing_buffer_Check (g_pChewingCtx))
    {
        pszTmp = chewing_buffer_String (g_pChewingCtx);
        g_himeModMainFuncs.mf_send_text (pszTmp);
        free (pszTmp);
    }

    chewing_Reset (g_pChewingCtx);

    // FIXME: dirty workaround to reset the libchewing internal data
    //        it may impact the bEscCleanAllBuf setting
    chewing_handle_Esc (g_pChewingCtx);

    hime_label_clear (MAX_SEG_NUM);

    if (*g_himeModMainFuncs.mf_hime_pop_up_win && is_empty())
        module_hide_win();

    return 0;
}

