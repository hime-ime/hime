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

#include "gtab.h"
#include "lang.h"
#include "pho.h"
#include "tsin.h"

char txt[128];

extern char *current_tsin_fname;
typedef unsigned int u_int32_t;

#define PAGE_LEN 20

void init_gtab (int inmdno);
gboolean init_tsin_table_fname (INMD *p, char *fname);
void load_tsin_db0 (char *infname, gboolean is_gtab_i);

GtkWidget *vbox_top;
INMD *pinmd;
char gtab_tsin_fname[256];
char is_gtab;

char *phokey2pinyin (phokey_t k);
gboolean b_pinyin;
GtkWidget *hbox_buttons;
char current_str[MAX_PHRASE_LEN * CH_SZ + 1];

GtkWidget *mainwin;

static int *ts_idx;
int tsN;
int page_ofs, select_cursor;
GtkWidget *labels[PAGE_LEN];
GtkWidget *button_check[PAGE_LEN];
GtkWidget *last_row, *find_textentry, *label_page_ofs;
int del_ofs[1024];
int del_ofsN;

void cp_ph_key (void *in, int idx, void *dest) {
    if (ph_key_sz == 2) {
        phokey_t *pharr = (phokey_t *) in;
        in = &pharr[idx];
    } else if (ph_key_sz == 4) {
        u_int32_t *pharr4 = (u_int32_t *) in;
        in = &pharr4[idx];
    } else {
        u_int64_t *pharr8 = (u_int64_t *) in;
        in = &pharr8[idx];
    }

    memcpy (dest, in, ph_key_sz);
}

void *get_ph_key_ptr (void *in, int idx) {
    if (ph_key_sz == 2) {
        phokey_t *pharr = (phokey_t *) in;
        return &pharr[idx];
    }

    if (ph_key_sz == 4) {
        u_int32_t *pharr4 = (u_int32_t *) in;
        return &pharr4[idx];
    }

    u_int64_t *pharr8 = (u_int64_t *) in;
    return &pharr8[idx];
}

int lookup_gtab_key (char *ch, void *out) {
    int outN = 0;
    INMD *tinmd = &inmd[default_input_method];

    for (int i = 0; i < tinmd->DefChars; i++) {
        char *chi = (char *) tblch2 (tinmd, i);

        if (!(chi[0] & 0x80)) {
            continue;
        }
        if (!utf8_eq (chi, ch)) {
            continue;
        }

        u_int64_t key = CONVT2 (tinmd, i);
        if (ph_key_sz == 4) {
            u_int32_t key32 = (u_int32_t) key;
            memcpy (get_ph_key_ptr (out, outN), &key32, ph_key_sz);
        } else {
            memcpy (get_ph_key_ptr (out, outN), &key, ph_key_sz);
        }
        outN++;
    }

    return outN;
}

extern FILE *fph;

void load_ts_phrase (void) {
    FILE *fp = fph;

    dbg ("fname %s\n", current_tsin_fname);

    int ofs = is_gtab ? sizeof (TSIN_GTAB_HEAD) : 0;
    fseek (fp, ofs, SEEK_SET);

    tsN = 0;
    free (ts_idx);
    ts_idx = NULL;

    while (!feof (fp)) {
        ts_idx = trealloc (ts_idx, int, tsN);
        ts_idx[tsN] = ftell (fp);
        u_int64_t phbuf[MAX_PHRASE_LEN];
        char chbuf[MAX_PHRASE_LEN * CH_SZ + 1];
        u_char clen = 0;
        usecount_t usecount = 0;

        fread (&clen, 1, 1, fp);

        if (clen > MAX_PHRASE_LEN) {
            p_err ("bad tsin db clen %d > MAX_PHRASE_LEN %d\n",
                   clen,
                   MAX_PHRASE_LEN);
        }

        fread (&usecount, sizeof (usecount_t), 1, fp);
        fread (phbuf, ph_key_sz, clen, fp);
        int tlen = 0;

        for (int i = 0; i < clen; i++) {
            int n = fread (&chbuf[tlen], 1, 1, fp);
            if (n <= 0) {
                goto stop;
            }
            int len = utf8_sz (&chbuf[tlen]);
            fread (&chbuf[tlen + 1], 1, len - 1, fp);
            tlen += len;
        }

        if (clen < 2) {
            continue;
        }

        chbuf[tlen] = 0;
        tsN++;
    }

    page_ofs = tsN - PAGE_LEN;

stop:
    dbg ("load_ts_phrase\n");
    //  fclose(fp);
}

int gtab_key2name (INMD *tinmd, u_int64_t key, char *t, int *rtlen);
void get_key_str (void *key, int idx, char *out_str) {
    char t[128];
    char *phostr = NULL;

    if (is_gtab) {
        int tlen = 0;
        u_int64_t key64 = 0;
        if (ph_key_sz == 4) {
            u_int32_t key32 = 0;
            cp_ph_key (key, idx, &key32);
            key64 = key32;
        } else {
            cp_ph_key (key, idx, &key64);
        }
        gtab_key2name (pinmd, key64, t, &tlen);
        phostr = t;
    } else {
        phokey_t k = 0;
        cp_ph_key (key, idx, &k);
        phostr = b_pinyin ? phokey2pinyin (k) : phokey_to_str (k);
    }

    strcpy (out_str, phostr);
}

void load_tsin_entry0 (char *len, usecount_t *usecount, void *pho, u_char *ch);

void load_tsin_at_ts_idx (int ts_row,
                          char *len,
                          usecount_t *usecount,
                          void *pho,
                          u_char *ch) {
    int ofs = ts_idx[ts_row];
    fseek (fph, ofs, SEEK_SET);

    load_tsin_entry0 (len, usecount, pho, ch);
}

void disp_page (void) {
    for (int li = 0; li < PAGE_LEN; li++) {
        char line[256];
        int ts_row = page_ofs + li;
        if (ts_row >= tsN) {
            gtk_label_set_text (GTK_LABEL (labels[li]), "-");
            gtk_widget_hide (button_check[li]);
            continue;
        }

        u_int64_t phbuf[MAX_PHRASE_LEN];
        char chbuf[MAX_PHRASE_LEN * CH_SZ + 1];
        char clen = 0;
        usecount_t usecount = 0;

        load_tsin_at_ts_idx (ts_row, &clen, &usecount, phbuf, (u_char *) chbuf);

        char *t = g_markup_escape_text (chbuf, -1);
        strcpy (line, t);
        g_free (t);
        strcat (line, " <span foreground=\"blue\">");

        char tt[512];

        for (int i = 0; i < clen; i++) {
            get_key_str (phbuf, i, tt);
            //      dbg("tt %s\n", tt);
            strcat (line, t = g_markup_escape_text (tt, -1));
            g_free (t);
            strcat (line, " ");
        }

        snprintf (tt, sizeof (tt), " %d", usecount);
        strcat (line, tt);
        strcat (line, "</span>");

        //    dbg("%s\n", line);
        gtk_label_set_markup (GTK_LABEL (labels[li]), line);
        gtk_widget_show (button_check[li]);
    }

    char tt[32];
    snprintf (tt, sizeof (tt), "%d", page_ofs + 1);
    gtk_label_set_text (GTK_LABEL (label_page_ofs), tt);
}

static void cb_button_delete (GtkButton *button, gpointer user_data) {
    for (int i = 0; i < PAGE_LEN; i++) {
        if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_check[i]))) {
            continue;
        }
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_check[i]),
                                      FALSE);

        del_ofs[del_ofsN++] = ts_idx[page_ofs + i];
        ts_idx[page_ofs + i] = -1;
    }

    int ntsN = 0;

    for (int i = 0; i < tsN; i++) {
        if (ts_idx[i] >= 0) {
            ts_idx[ntsN++] = ts_idx[i];
        }
    }
    tsN = ntsN;

    disp_page ();
}

static void cb_button_find_ok (GtkButton *button, gpointer user_data) {
    txt[0] = 0;
    strcpy (txt, gtk_entry_get_text (GTK_ENTRY (find_textentry)));
    //  gtk_widget_destroy(last_row);
    //  last_row = NULL;
    if (!txt[0]) {
        return;
    }
    int row = page_ofs + 1;
    for (; row < tsN; row++) {
        u_int64_t phbuf[MAX_PHRASE_LEN];
        char chbuf[MAX_PHRASE_LEN * CH_SZ + 1];
        char clen = 0;
        usecount_t usecount = 0;

        load_tsin_at_ts_idx (row, &clen, &usecount, phbuf, (u_char *) chbuf);
        if (strstr (chbuf, txt)) {
            break;
        }
    }

    if (row == tsN) {
        GtkWidget *dia = gtk_message_dialog_new (NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_INFO,
                                                 GTK_BUTTONS_OK,
                                                 "%s not found",
                                                 txt);
        gtk_dialog_run (GTK_DIALOG (dia));
        gtk_widget_destroy (dia);
    } else {
        page_ofs = row;
        disp_page ();
    }
}

static void cb_button_close (GtkButton *button, gpointer user_data) {
    if (last_row) {
        gtk_widget_destroy (last_row);
    }
    last_row = NULL;
}

static void cb_button_find (GtkButton *button, gpointer user_data) {
    if (last_row) {
        gtk_widget_destroy (last_row);
    }
    last_row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *lab = gtk_label_new ("Find");
    gtk_box_pack_start (GTK_BOX (last_row), lab, FALSE, FALSE, 0);
    find_textentry = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (last_row), find_textentry, FALSE, FALSE, 5);
    GtkWidget *button_ok = gtk_button_new_from_stock (GTK_STOCK_OK);
    g_signal_connect (G_OBJECT (button_ok), "clicked",
                      G_CALLBACK (cb_button_find_ok), NULL);
    gtk_box_pack_start (GTK_BOX (last_row), button_ok, FALSE, FALSE, 5);

    GtkWidget *button_close = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
    g_signal_connect (G_OBJECT (button_close), "clicked",
                      G_CALLBACK (cb_button_close), NULL);
    gtk_box_pack_start (GTK_BOX (last_row), button_close, FALSE, FALSE, 5);

    gtk_box_pack_start (GTK_BOX (vbox_top), last_row, FALSE, FALSE, 0);

    gtk_entry_set_text (GTK_ENTRY (find_textentry), txt);

    gtk_widget_show_all (last_row);
}

static void cb_button_save (GtkButton *button, gpointer user_data) {
    for (int i = 0; i < del_ofsN; i++) {
        fseek (fph, del_ofs[i], SEEK_SET);
        signed char clen = 0;
        fread (&clen, 1, 1, fph);
        if (clen > 0) {
            clen = -clen;
            fseek (fph, del_ofs[i], SEEK_SET);
            fwrite (&clen, 1, 1, fph);
        }
    }
    fflush (fph);

    unix_exec (HIME_BIN_DIR "/hime-tsd2a32 %s -o tsin.tmp", current_tsin_fname);
    unix_exec (HIME_BIN_DIR "/hime-tsa2d32 tsin.tmp %s", current_tsin_fname);
    exit (0);
}

#define MAX_SAME_CHAR_PHO (16)

typedef struct {
    u_int64_t phokeys[MAX_SAME_CHAR_PHO];
    int phokeysN;
    GtkWidget *opt_menu;
} char_pho;

static char_pho bigpho[MAX_PHRASE_LEN];
static int bigphoN;

static GtkWidget *hbox_pho_sel;

void destroy_pho_sel_area (void) {
    gtk_widget_destroy (hbox_pho_sel);
}

static void cb_button_ok (GtkButton *button, gpointer user_data) {
    u_int64_t pharr8[MAX_PHRASE_LEN];

    for (int i = 0; i < bigphoN; i++) {
        int idx = gtk_combo_box_get_active (GTK_COMBO_BOX (bigpho[i].opt_menu));
        void *dest = get_ph_key_ptr (pharr8, i);

        cp_ph_key (bigpho[i].phokeys, idx, dest);
    }

    save_phrase_to_db (pharr8, current_str, bigphoN, 0);

    destroy_pho_sel_area ();
}

static void cb_button_cancel (GtkButton *button, gpointer user_data) {
}

GtkWidget *create_pho_sel_area (void) {
    hbox_pho_sel = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    for (int i = 0; i < bigphoN; i++) {
        bigpho[i].opt_menu = gtk_combo_box_text_new ();
        gtk_box_pack_start (GTK_BOX (hbox_pho_sel),
                            bigpho[i].opt_menu,
                            FALSE,
                            FALSE,
                            0);

        for (int j = 0; j < bigpho[i].phokeysN; j++) {
            char t[128];
            char *phostr = NULL;

            if (is_gtab) {
                int tlen = 0;
                u_int64_t key64 = 0;
                if (ph_key_sz == 4) {
                    u_int32_t key32 = 0;
                    cp_ph_key (bigpho[i].phokeys, j, &key32);
                    key64 = key32;
                } else {
                    cp_ph_key (bigpho[i].phokeys, j, &key64);
                }

                gtab_key2name (pinmd, key64, t, &tlen);
                phostr = t;
            } else {
                phokey_t k = 0;
                cp_ph_key (bigpho[i].phokeys, j, &k);
                phostr = b_pinyin ? phokey2pinyin (k) : phokey_to_str (k);
            }

            gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (bigpho[i].opt_menu), phostr);
        }

        gtk_combo_box_set_active (GTK_COMBO_BOX (bigpho[i].opt_menu), 0);
    }

    GtkWidget *button_ok = gtk_button_new_with_label ("OK to add");
    gtk_box_pack_start (GTK_BOX (hbox_pho_sel), button_ok, FALSE, FALSE, 20);
    g_signal_connect (G_OBJECT (button_ok), "clicked",
                      G_CALLBACK (cb_button_ok), NULL);

    GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    gtk_box_pack_start (GTK_BOX (hbox_pho_sel), button_cancel, FALSE, FALSE, 20);
    g_signal_connect (G_OBJECT (button_cancel), "clicked",
                      G_CALLBACK (cb_button_cancel), NULL);

    return hbox_pho_sel;
}

static Display *display;

void do_exit (void) {
    send_hime_message (display, RELOAD_TSIN_DB);
    exit (0);
}

void load_tsin_db ();
void set_window_hime_icon (GtkWidget *window);

#if 0
static gboolean  scroll_event(GtkWidget *widget,GdkEventScroll *event, gpointer user_data)
{
  dbg("scroll_event\n");

  if (event->direction!=GDK_SCROLL_DOWN)
    return TRUE;

  return FALSE;
}
#endif

static void prev_item (void) {
    if (page_ofs > 0) {
        page_ofs--;
    }
}

static void next_item (void) {
    if (page_ofs < tsN - 1) {
        page_ofs++;
    }
}

static void page_up (void) {
    page_ofs -= PAGE_LEN;
    if (page_ofs < 0) {
        page_ofs = 0;
    }
}

static void page_down (void) {
    page_ofs += PAGE_LEN;
    if (page_ofs >= tsN) {
        page_ofs = tsN - 1;
    }
}

gboolean key_press_event (GtkWidget *widget,
                          GdkEventKey *event,
                          gpointer user_data) {
    if (last_row) {
        return FALSE;
    }

    switch (event->keyval) {
    case GDK_KEY_Up:
        prev_item ();
        break;
    case GDK_KEY_Down:
        next_item ();
        break;
    case GDK_KEY_Page_Up:
        page_up ();
        break;
    case GDK_KEY_Page_Down:
        page_down ();
        break;
    case GDK_KEY_Home:
        page_ofs = 0;
        break;
    case GDK_KEY_End:
        page_ofs = tsN - PAGE_LEN;
        break;
    }

    disp_page ();

    return TRUE;
}

gboolean is_pinyin_kbm ();

int main (int argc, char **argv) {
    set_is_chs ();
    init_TableDir ();
    b_pinyin = is_pinyin_kbm ();

    gtk_init (&argc, &argv);
    load_settings ();
    load_gtab_list (TRUE);

    char hime_dir[512];
    get_hime_dir (hime_dir);
    chdir (hime_dir);

#if HIME_I18N_MESSAGE
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif

    pinmd = &inmd[default_input_method];

    if (pinmd->method_type == method_type_TSIN) {
        dbg ("is tsin\n");
        pho_load ();
        load_tsin_db ();
        ph_key_sz = 2;
    } else if (pinmd->filename) {
        dbg ("gtab filename %s\n", pinmd->filename);
        init_gtab (default_input_method);
        is_gtab = TRUE;
        init_tsin_table_fname (pinmd, gtab_tsin_fname);
        load_tsin_db0 (gtab_tsin_fname, TRUE);
    } else {
        p_err ("Your default input method %s doesn't use phrase database",
               pinmd->cname);
    }

    dbg ("ph_key_sz: %d\n", ph_key_sz);

    display = GDK_DISPLAY ();

    mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position (GTK_WINDOW (mainwin), GTK_WIN_POS_CENTER);

    g_signal_connect (G_OBJECT (mainwin), "key-press-event",
                      G_CALLBACK (key_press_event), NULL);

    gtk_window_set_has_resize_grip (GTK_WINDOW (mainwin), FALSE);
    gtk_window_set_resizable (GTK_WINDOW (mainwin), FALSE);
    //  gtk_window_set_default_size(GTK_WINDOW (mainwin), 640, 520);
    set_window_hime_icon (mainwin);

    vbox_top = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_orientable_set_orientation (GTK_ORIENTABLE (vbox_top),
                                    GTK_ORIENTATION_VERTICAL);
    gtk_container_add (GTK_CONTAINER (mainwin), vbox_top);

    for (int i = 0; i < PAGE_LEN; i++) {
        GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start (GTK_BOX (vbox_top), hbox, FALSE, FALSE, 0);
        button_check[i] = gtk_check_button_new ();
        gtk_box_pack_start (GTK_BOX (hbox), button_check[i], FALSE, FALSE, 0);

        labels[i] = gtk_label_new (NULL);
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_halign (hbox, GTK_ALIGN_START);
        gtk_widget_set_valign (hbox, GTK_ALIGN_START);
        gtk_container_add (GTK_CONTAINER (hbox), labels[i]);
#else
        GtkWidget *align = gtk_alignment_new (0, 0, 0, 0);
        gtk_container_add (GTK_CONTAINER (align), labels[i]);
        gtk_box_pack_start (GTK_BOX (hbox), align, FALSE, FALSE, 0);
#endif
    }

    hbox_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_buttons, FALSE, FALSE, 0);

    GtkWidget *button_delete = gtk_button_new_from_stock (GTK_STOCK_DELETE);
    gtk_box_pack_start (GTK_BOX (hbox_buttons), button_delete, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_delete), "clicked",
                      G_CALLBACK (cb_button_delete), NULL);

    GtkWidget *button_find = gtk_button_new_from_stock (GTK_STOCK_FIND);
    gtk_box_pack_start (GTK_BOX (hbox_buttons), button_find, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_find), "clicked",
                      G_CALLBACK (cb_button_find), NULL);

    GtkWidget *button_save = gtk_button_new_from_stock (GTK_STOCK_SAVE);
    gtk_box_pack_start (GTK_BOX (hbox_buttons), button_save, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_save), "clicked",
                      G_CALLBACK (cb_button_save), NULL);

    GtkWidget *button_quit = gtk_button_new_from_stock (GTK_STOCK_QUIT);
    gtk_box_pack_start (GTK_BOX (hbox_buttons), button_quit, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_quit), "clicked",
                      G_CALLBACK (do_exit), NULL);

    label_page_ofs = gtk_label_new (NULL);
    gtk_box_pack_start (GTK_BOX (hbox_buttons), label_page_ofs, FALSE, FALSE, 3);

    g_signal_connect (G_OBJECT (mainwin), "delete-event",
                      G_CALLBACK (do_exit), NULL);

    gtk_widget_show_all (mainwin);

    load_ts_phrase ();

    disp_page ();

    gtk_main ();
    return 0;
}
