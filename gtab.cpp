/* Copyright (C) 2004-2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include <sys/stat.h>
#include <regex.h>
#include "hime.h"
#include "gtab.h"
#include "pho.h"
#include "hime-conf.h"
#include "hime-endian.h"
#include "gtab-buf.h"
#include "tsin.h"
#include "gst.h"

extern gboolean test_mode;
gboolean gtab_phrase_on();
void init_seltab(char ***p);

extern gint64 key_press_time, key_press_time_ctrl;

extern GtkWidget *gwin_gtab;
void hide_gtab_pre_sel();
void gtab_scan_pre_select(gboolean);

extern GTAB_space_pressed_E _gtab_space_auto_first;
extern char *TableDir;

extern INMD *cur_inmd;
extern char **seltab;

#define gtab_full_space_auto_first (_gtab_space_auto_first & (GTAB_space_auto_first_any|GTAB_space_auto_first_full))
#define AUTO_SELECT_BY_PHRASE (gtab_phrase_on())

gboolean use_tsin_sel_win()
{
  return
//  !gtab_pre_select && !gtab_disp_partial_match &&
  gtab_vertical_select && gtab_phrase_pre_select;
}

static gboolean disp_partial_match_on()
{
 return gtab_disp_partial_match || (cur_inmd->flag&FLAG_GTAB_DISP_PARTIAL_MATCH)!=0;
}


static gboolean gtab_pre_select_on()
{
 return gtab_pre_select || (cur_inmd->flag&FLAG_GTAB_DISP_PARTIAL_MATCH)!=0;
}

gboolean same_query_show_pho_win()
{
  return poo.same_pho_query_state != SAME_PHO_QUERY_none;
}

gboolean hime_edit_display_ap_only();
gboolean gtab_has_input()
{
  int i;

  for(i=0; i < MAX_TAB_KEY_NUM64_6; i++)
    if (ggg.inch[i])
      return TRUE;

  if (same_query_show_pho_win())
    return TRUE;

  if (ggg.gtab_buf_select)
    return TRUE;

  if (ggg.gbufN && !hime_edit_display_ap_only())
    return TRUE;

  return FALSE;
}

#define tblch(i) tblch2(cur_inmd, i)

int load_phr_ch(INMD *inm, u_char *ch, char *tt)
{
  int phrno =((int)(ch[0])<<16)|((int)ch[1]<<8)|ch[2];
  int ofs = inm->phridx[phrno], ofs1 = inm->phridx[phrno+1];

//  dbg("load_phr   j:%d %d %d %d\n", j, phrno, ofs, ofs1);
  int len = ofs1 - ofs;

  if (len > MAX_CIN_PHR || len <= 0) {
    dbg("phrae error %d\n", len);
    strcpy(tt,"err");
    return 0;
  }

  memcpy(tt, inm->phrbuf + ofs, len);
  tt[len]=0;
  return len;
}

static void load_phr(int j, char *tt)
{
  u_char *ch = tblch(j);

  load_phr_ch(cur_inmd, ch, tt);
}

static int qcmp_strlen(const void *aa, const void *bb)
{
  char *a = *((char **)aa), *b = *((char **)bb);

  return strlen(a) - strlen(b);
}

void set_key_codes_label(char *s, int better);
void set_page_label(char *s);

static void clear_page_label()
{
  set_page_label("");
}

int gtab_key2name(INMD *tinmd, u_int64_t key, char *t, int *rtlen);


int ch_to_gtab_keys(INMD *tinmd, char *ch, u_int64_t keys[])
{
  int n = utf8_str_N(ch);
  gboolean phrase = n > 1 || !(ch[0] & 0x80);
  int i, keysN=0;
  for(i=0; i < tinmd->DefChars; i++) {
    char *chi = (char *)tblch2(tinmd, i);

    if (phrase) {
      if ((chi[0] & 0x80))
        continue;
      char tstr[512];
      load_phr_ch(tinmd, (u_char *)chi, tstr);
      if (strcmp(tstr, ch))
        continue;
    } else {
      if (!(chi[0] & 0x80))
        continue;
      if (!utf8_eq(chi, ch))
        continue;
    }

    u_int64_t key = CONVT2(tinmd, i);
    keys[keysN++] = key;
  }
  return keysN;
}

void lookup_gtabn(char *ch, char *out)
{
  char outbuf[512];
  char *tbuf[128];
  int tbufN=0;
  INMD *tinmd = &inmd[default_input_method];

  if (!tinmd->DefChars)
    tinmd = cur_inmd;

  if (!tinmd)
    return;

  gboolean need_disp = FALSE;

  if (!out) {
    out = outbuf;
    need_disp = TRUE;
  }

  out[0]=0;


  int min_klen = 100;
  u_int64_t keys[64];
  int keysN = ch_to_gtab_keys(tinmd, ch, keys);

  int i;
  for(i=0; i < keysN; i++) {
    int tlen, klen;
    char t[CH_SZ * 10 + 1];

    klen = gtab_key2name(tinmd, keys[i], t, &tlen);

    if (klen < min_klen)
      min_klen = klen;

    t[tlen]=0;

    tbuf[tbufN] = strdup(t);
    tbufN++;
  }


  qsort(tbuf, tbufN, sizeof(char *), qcmp_strlen);
  out[0]=0;

  for(i=0; i < tbufN; i++) {
#define MAX_DISP_MATCH 40
    if (strlen(out) < MAX_DISP_MATCH) {
      strcat(out, tbuf[i]);
      if (i < tbufN-1)
        strcat(out, " |");
    }

    free(tbuf[i]);
  }

  if (!out[0] || !need_disp)
    return;


  set_key_codes_label(out, ggg.ci > min_klen);
  void set_key_codes_label_pho(char *s);
  set_key_codes_label_pho(out);
}

void lookup_gtab(char *ch)
{
  char tt[CH_SZ+1];
  utf8cpy(tt, ch);
  lookup_gtabn(tt, NULL);
}


void lookup_gtab_out(char *ch, char *out)
{
  char tt[CH_SZ+1];
  utf8cpy(tt, ch);
  lookup_gtabn(tt, out);
}

void free_gtab()
{
  int i;

  for(i=0; i < inmdN; i++) {
    INMD *inp = &inmd[i];
    free(inp->tbl); inp->tbl = NULL;
    free(inp->tbl64); inp->tbl64 = NULL;
    free(inp->phridx); inp->phridx = NULL;
    free(inp->phrbuf); inp->phrbuf = NULL;
    free(inp->keyname_lookup); inp->keyname_lookup = NULL;
  }

  free(inmd);
}


char *b1_cat(char *s, char c)
{
  char t[2];
  t[0]=c;
  t[1]=0;

  return strcat(s, t);
}


char *bch_cat(char *s, char *ch)
{
  char t[CH_SZ + 1];
  int len = u8cpy(t, ch);
  t[len]=0;

  return strcat(s, t);
}


void minimize_win_gtab();
void disp_gtab_sel(char *s);

void ClrSelArea()
{
  disp_gtab_sel("");
  minimize_win_gtab();
//  hide_gtab_pre_sel();
}


void disp_gtab(char *);
void clear_gtab_input_error_color();

static void clr_seltab()
{
  int i;
  if (!seltab)
    return;

  for(i=0; i < MAX_SELKEY; i++)
     seltab[i][0]=0;
}

void clear_gtab_in_area(), hide_win_gtab();
void ClrIn()
{
#if WIN32
  if (test_mode)
    return;
#endif

  bzero(ggg.inch,sizeof(ggg.inch));
  clr_seltab();
  ggg.total_matchN=ggg.pg_idx=ggg.more_pg=ggg.wild_mode=ggg.wild_page=ggg.last_idx=ggg.defselN=ggg.exa_match=
  ggg.spc_pressed=ggg.ci=ggg.invalid_spc=0;

  ggg.sel1st_i=MAX_SELKEY-1;

  clear_gtab_in_area();
  ggg.last_idx = 0;

  if (hime_pop_up_win && !gtab_has_input() && !tss.pre_selN)
    hide_win_gtab();

  clear_gtab_input_error_color();
  clear_page_label();
//  hide_gtab_pre_sel();
}


void hide_win_pho();

void close_gtab_pho_win()
{
  if (test_mode)
    return;
  if (poo.same_pho_query_state != SAME_PHO_QUERY_none) {
    poo.same_pho_query_state = SAME_PHO_QUERY_none;
    hide_win_pho();
  }
}

void gtab_disp_empty(char *tt, int N);
extern int win_gtab_max_key_press;

static void DispInArea()
{
  int i;
#if WIN32
  if (test_mode)
    return;
#endif

//  hide_gtab_pre_sel();

//  dbg("sel1st:%d\n", ggg.sel1st_i);
  if (hime_display_on_the_spot_key()) {
    if (gwin_gtab && GTK_WIDGET_VISIBLE(gwin_gtab) && poo.same_pho_query_state == SAME_PHO_QUERY_none)
      hide_win_gtab();
    return;
  }

  char tt[128];
  int ttN=0;

  if (win_gtab_max_key_press < ggg.ci)
    win_gtab_max_key_press = ggg.ci;

  for(i=0;i<ggg.ci;i++) {
    char *p=(char *)&cur_inmd->keyname[ggg.inch[i] * CH_SZ];
    int len;
    if (*p & 0x80)
      len=utf8cpy(tt+ttN, p);
    else {
      len = strlen(p);
      strcpy(tt+ttN, p);
    }

    ttN+=len;
  }

  tt[ttN]=0;

  gtab_disp_empty(tt, win_gtab_max_key_press - i);

  disp_gtab(tt);
  minimize_win_gtab();
}

int get_DispInArea_str(char *out)
{
  int outN=0, i;
  for(i=0;i<ggg.ci;i++) {
    char *p = (char *)&cur_inmd->keyname[ggg.inch[i] * CH_SZ];
    if (*p & 0x80)
      outN+=u8cpy(out+outN, p);
    else {
      int len = strlen(p);
      memcpy(out+outN, p, len);
      outN+=len;
    }
  }

#if 0
  if (outN) {
    hide_gtab_pre_sel();
  }
#endif

  out[outN]=0;
//  dbg("get_DispInArea_str\n", out);
  return outN;
}


void set_gtab_input_method_name(char *s);
void case_inverse(KeySym *xkey, int shift_m);

extern unich_t *fullchar[];

void start_gtab_pho_query(char *utf8);

void clear_after_put()
{
  ClrIn();
  ClrSelArea();
}

void add_to_tsin_buf_str(char *str);
gboolean init_in_method(int in_no);
void hide_win_kbm();

void hide_row2_if_necessary()
{
  if (!ggg.wild_mode && gtab_hide_row2 || !gtab_disp_key_codes) {
    set_key_codes_label(NULL, 0);
  }
}

static void putstr_inp(char *p)
{
#if WIN32
  if (test_mode)
    return;
#endif

  clear_page_label();

//  dbg("gtab_hide_row2 %d\n", gtab_hide_row2);
  hide_row2_if_necessary();

  char_play(p);

  int to_tsin = (cur_inmd->flag & FLAG_GTAB_SYM_KBM) && inmd[default_input_method].method_type==method_type_TSIN && tss.c_len;

  if (utf8_str_N(p) > 1  || !(p[0]&128)) {
    if (gtab_disp_key_codes && !gtab_hide_row2 || ggg.wild_mode)
      lookup_gtabn(p, NULL);
#if USE_TSIN
    if (to_tsin) {
      add_to_tsin_buf_str(p);
    }
    else
#endif
      send_text(p);
  }
  else {
    if (poo.same_pho_query_state == SAME_PHO_QUERY_gtab_input) {
      poo.same_pho_query_state = SAME_PHO_QUERY_pho_select;
      start_gtab_pho_query(p);

      ClrIn();
      ClrSelArea();
      return;
    }

    if (gtab_disp_key_codes && !gtab_hide_row2 || ggg.wild_mode)
      lookup_gtab(p);

    if (to_tsin)
      add_to_tsin_buf_str(p);
    else
      send_utf8_ch(p);
  }

  clear_after_put();

  if ((cur_inmd->flag & FLAG_GTAB_SYM_KBM)) {
    extern int win_kbm_inited, b_show_win_kbm;
    init_in_method(default_input_method);
    if (win_kbm_inited && !b_show_win_kbm)
      hide_win_kbm();
  }
}


#define swap(a,b) { tt=a; a=b; b=tt; }

static u_int vmask[]=
{ 0,
 (0x3f<<24),
 (0x3f<<24)|(0x3f<<18),
 (0x3f<<24)|(0x3f<<18)|(0x3f<<12),
 (0x3f<<24)|(0x3f<<18)|(0x3f<<12)|(0x3f<<6),
 (0x3f<<24)|(0x3f<<18)|(0x3f<<12)|(0x3f<<6)|0x3f
};


static u_int vmask_7[]=
{ 0,
 (0x7f<<21),
 (0x7f<<21)|(0x7f<<14),
 (0x7f<<21)|(0x7f<<14)|(0x7f<<7),
 (0x7f<<21)|(0x7f<<14)|(0x7f<<7)|0x7f,
};

#define KKK ((u_int64_t)0x3f)


static u_int64_t vmask64[]=
{ 0,
  (KKK<<54),
  (KKK<<54)|(KKK<<48),
  (KKK<<54)|(KKK<<48)|(KKK<<42),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30)|(KKK<<24),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30)|(KKK<<24)|(KKK<<18),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30)|(KKK<<24)|(KKK<<18)|(KKK<<12),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30)|(KKK<<24)|(KKK<<18)|(KKK<<12)|(KKK<<6),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30)|(KKK<<24)|(KKK<<18)|(KKK<<12)|(KKK<<6)|KKK
};


#define KKK7 ((u_int64_t)0x7f)

static u_int64_t vmask64_7[]=
{ 0,
 (KKK7<<56),
 (KKK7<<56)|(KKK7<<49),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35)|(KKK7<<28),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35)|(KKK7<<28)|(KKK7<<21),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35)|(KKK7<<28)|(KKK7<<21)|(KKK7<<14),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35)|(KKK7<<28)|(KKK7<<21)|(KKK7<<14)|(KKK7<<7),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35)|(KKK7<<28)|(KKK7<<21)|(KKK7<<14)|(KKK7<<7)|KKK7,
};


#define KEY_N (cur_inmd->max_keyN)

static gboolean load_seltab(int tblidx, int seltabidx)
{
  u_char *tbl_ch = tblch(tblidx);
  if (tbl_ch[0] < 0x80) {
    load_phr(tblidx, seltab[seltabidx]);
    return TRUE;
  }

  int len = u8cpy(seltab[seltabidx], (char *)tbl_ch);
  seltab[seltabidx][len] = 0;

  return FALSE;
}


static char* load_tblidx(int tblidx)
{
  char tt[MAX_CIN_PHR];
  u_char *tbl_ch = tblch(tblidx);
  if (tbl_ch[0] < 0x80) {
    load_phr(tblidx, tt);
  } else {
    int len = u8cpy(tt, (char *)tbl_ch);
    tt[len] = 0;
  }

  return strdup(tt);
}


void set_gtab_input_error_color();
static void bell_err()
{
  if (test_mode)
    return;

  bell();
  set_gtab_input_error_color();
}

gboolean cmp_inmd_idx(regex_t *reg, int idx)
{
  u_int64_t kk=CONVT2(cur_inmd, idx);
  char ts[32];
  int tsN=0;

  ts[tsN++]= ' ';

  int i;
  for(i=0; i < KEY_N; i++) {
    char c = (kk >> (LAST_K_bitN - i*cur_inmd->keybits)) & cur_inmd->kmask;
    if (!c)
      break;
    ts[tsN++] = c + '0';
  }

  ts[tsN++]= ' ';
  ts[tsN]=0;

  return regexec(reg, ts, 0, 0, 0);
}

int page_len()
{
  return (_gtab_space_auto_first & GTAB_space_auto_first_any) ?
  cur_inmd->M_DUP_SEL+1:cur_inmd->M_DUP_SEL;
}

static void page_no_str(char tstr[])
{
  if (ggg.wild_mode || ggg.gtab_buf_select) {
    int pgN = (ggg.total_matchN + cur_inmd->M_DUP_SEL - 1) / cur_inmd->M_DUP_SEL;
    if (pgN < 2)
      return;

    int pg = ggg.gtab_buf_select ? ggg.pg_idx : ggg.wild_page;
    sprintf(tstr, "%d/%d", pg /cur_inmd->M_DUP_SEL + 1, pgN);
  } else {
    int pgN = (ggg.E1 - ggg.S1 + page_len() - 1) /page_len();

    if (pgN < 2)
      return;

    sprintf(tstr, "%d/%d", (ggg.pg_idx - ggg.S1)/page_len()+1, pgN);
  }
}

char *htmlspecialchars(char *s, char out[])
{
  struct {
    char c;
    char *str;
  } chs[]= {{'>',"gt"}, {'<',"lt"}, {'&',"amp"}
#if 0
  , {' ',"nbsp"}
#endif
  };
  int chsN=sizeof(chs)/sizeof(chs[0]);

  int outn=0;
  while (*s) {
    int sz = utf8_sz(s);
    int i;
    for(i=0; i<chsN; i++)
      if (chs[i].c==*s)
        break;
    if (i==chsN) {
      memcpy(&out[outn],s, sz);
      outn+=sz;
      s+=sz;
    }
    else {
      out[outn++]='&';
      int len=strlen(chs[i].str);
      memcpy(&out[outn], chs[i].str, len);
      outn+=len;
      out[outn++]=';';
      s++;
    }
  }

  out[outn]=0;
  return out;
}


void disp_selection0(gboolean phrase_selected, gboolean force_disp)
{
  char pgstr[32];
  pgstr[0]=0;
  page_no_str(pgstr);

  if (!gtab_vertical_select) {
    if (ggg.more_pg)
      set_page_label(pgstr);
    else
      clear_page_label();
  }

  char tt[1024];
  tt[0]=0;
  char uu[MAX_CIN_PHR];

  int ofs;
  if (!ggg.wild_mode && ggg.exa_match && (_gtab_space_auto_first & GTAB_space_auto_first_any)) {
    strcat(tt, htmlspecialchars(seltab[0], uu));
    if (gtab_vertical_select)
      strcat(tt, "\n");
    else
      strcat(tt, " ");
    ofs = 1;
  } else {
    ofs = 0;
  }


  int i,max_i;
  for(max_i = cur_inmd->M_DUP_SEL + ofs-1; max_i>=0; max_i--)
    if (seltab[max_i][0])
      break;

  for(i=ofs; i<= max_i; i++) {
    if (seltab[i][0]) {
      char selback[MAX_CIN_PHR+16];
      htmlspecialchars(seltab[i], selback);

      utf8cpy(uu, &cur_inmd->selkey[i - ofs]);
      char vvv[16];
      char www[1024];
      sprintf(www, "<span foreground=\"%s\">%s</span>", hime_sel_key_color, htmlspecialchars(uu, vvv));
      strcat(tt, www);

      if (gtab_vertical_select)
        strcat(tt, " ");

      if (phrase_selected && i==ggg.sel1st_i) {
        strcat(tt, "<span foreground=\"red\">");
        strcat(strcat(tt, selback), " ");
        strcat(tt, "</span>");
      } else {
        char uu[MAX_CIN_PHR];

        if (gtab_vertical_select) {
          utf8cpy_bytes(uu, selback, 120);
          strcat(tt, uu);
        } else {
          char *p = selback;

          static char *skip[]={"http://", "ftp://", "https://", NULL};

          int j;
          for(j=0; skip[j]; j++)
            if (!strncmp(seltab[i], skip[j], strlen(skip[j]))) {
              p+=strlen(skip[j]);
              break;
            }

          utf8cpy_bytes(uu, p, 6 * 3);
          strcat(strcat(tt, uu), " ");
        }
      }

      if (gtab_vertical_select)
        strcat(tt, "\n");
    } else {
      extern gboolean b_use_full_space;

      if (!gtab_vertical_select && disp_partial_match_on()) {
         if (b_use_full_space)
           strcat(tt, _(_L(" 　 ")));
         else {
           strcat(tt, "   ");
         }
      }
    }
  }

  if (gtab_vertical_select && pgstr[0]) {
    char tstr2[16];
    sprintf(tstr2, "(%s)", pgstr);
    strcat(tt, tstr2);
  }

  int len = strlen(tt);
  if (len && tt[len-1] == '\n')
    tt[len-1] = 0;

  if (gtab_pre_select_on() || ggg.wild_mode || ggg.spc_pressed || ggg.last_full || force_disp) {
    disp_gtab_sel(tt);
  }
}


void disp_selection(gboolean phrase_selected)
{
  disp_selection0(phrase_selected, FALSE);
}

void wildcard()
{
  int i,t, wild_ofs=0;
  int found=0;
  regex_t reg;

  ClrSelArea();
  clr_seltab();
  /* printf("wild %d %d %d %d\n", ggg.inch[0], ggg.inch[1], ggg.inch[2], ggg.inch[3]); */
  ggg.defselN=0;
  char regstr[32];
  int regstrN=0;

  regstr[regstrN++]=' ';

  for(i=0; i < KEY_N; i++) {
    if (!ggg.inch[i])
      break;
    if (ggg.inch[i] == cur_inmd->WILD_STAR) {
      regstr[regstrN++]='.';
      regstr[regstrN++]='*';
    } else
    if (ggg.inch[i] == cur_inmd->WILD_QUES) {
      regstr[regstrN++]='.';
    } else {
      char c = ggg.inch[i] + '0';         // start from '0'
      if (strchr("*.\\()[]", c))
      regstr[regstrN++] = '\\';
      regstr[regstrN++]=c;
    }
  }

  regstr[regstrN++]=' ';
  regstr[regstrN]=0;

//  dbg("regstr %s\n", regstr);

  if (regcomp(&reg, regstr, 0)) {
    dbg("regcomp failed\n");
    return;
  }

  for(t=0; t< cur_inmd->DefChars && ggg.defselN < cur_inmd->M_DUP_SEL; t++) {
    if (cmp_inmd_idx(&reg, t))
      continue;

    if (wild_ofs >= ggg.wild_page) {
      load_seltab(t, ggg.defselN);
      ggg.defselN++;
    } else
      wild_ofs++;

    found=1;
  } /* for t */


  if (!found) {
    bell_err();
  } else
  if (!ggg.wild_page) {
    ggg.total_matchN = 0;

    for(t=0; t< cur_inmd->DefChars; t++)
      if (!cmp_inmd_idx(&reg, t))
        ggg.total_matchN++;

  }

  if (ggg.total_matchN > cur_inmd->M_DUP_SEL)
    ggg.more_pg = 1;

  regfree(&reg);
  disp_selection(FALSE);
}

static char *ptr_selkey(KeySym key)
{
  if (key>= XK_KP_0 && key<= XK_KP_9)
    key-= XK_KP_0 - '0';
  return strchr(cur_inmd->selkey, key);
}


void init_gtab_pho_query_win();
int feedkey_pho(KeySym xkey, int state);

void set_gtab_target_displayed()
{
  close_gtab_pho_win();
}

gboolean is_gtab_query_mode()
{
  return poo.same_pho_query_state == SAME_PHO_QUERY_pho_select;
}

void reset_gtab_all()
{
  if (!cur_inmd)
    return;

  ClrIn();
  ClrSelArea();
}


static gboolean has_wild_card()
{
  int i;

  for(i=0; i < cur_inmd->MaxPress; i++)
    if (ggg.inch[i]>= cur_inmd->WILD_QUES) {
      return TRUE;
    }

  return FALSE;
}

static void proc_wild_disp()
{
   DispInArea();
   ggg.wild_page = 0;
   wildcard();
   disp_selection(0);
}

gboolean full_char_proc(KeySym keysym);
void insert_gbuf_cursor_char(char ch);
gboolean gtab_pre_select_shift(KeySym key, int kbstate);

gboolean shift_char_proc(KeySym key, int kbstate)
{
    if (key >= 127)
      return FALSE;

#if 0
    if (kbstate & LockMask) {
      if (key >= 'a' && key <= 'z')
        key-=0x20;
    } else {
      if (key >= 'A' && key <= 'Z')
        key+=0x20;
    }
#endif

    if (gtab_pre_select_shift(key, kbstate))
      return TRUE;

    if (current_CS->b_half_full_char)
      return full_char_proc(key);

    if (ggg.gbufN)
      insert_gbuf_cursor_char(key);
    else
      send_ascii(key);

    return TRUE;
}

extern GtkWidget *gwin_pho;
gboolean feed_phrase(KeySym ksym, int state);
int gtab_buf_backspace();
gboolean output_gbuf();
int show_buf_select();
void gbuf_next_pg(), gbuf_prev_pg();
void show_win_gtab();
int gbuf_cursor_left();
int gbuf_cursor_right();
int gbuf_cursor_home();
int gbuf_cursor_end();
int gtab_buf_delete();
void set_gbuf_c_sel(int v);
void set_gtab_user_head();
KeySym keypad_proc(KeySym xkey);
void save_gtab_buf_phrase(KeySym key);
gboolean save_gtab_buf_shift_enter();
gboolean win_sym_page_up(), win_sym_page_down();
u_int64_t vmaskci;
gboolean gtab_pre_select_idx(int c);
void save_CS_current_to_temp();

gboolean feedkey_gtab(KeySym key, int kbstate)
{
  int i,j=0;
  int inkey=0;
  char *pselkey= NULL;
  gboolean phrase_selected = FALSE;
  char seltab_phrase[MAX_SELKEY];
  gboolean is_keypad = FALSE;
  gboolean shift_m = (kbstate & ShiftMask) > 0;
//  gboolean ctrl_m = (kbstate & ControlMask) > 0;
  int caps_eng_tog = tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock;
  gboolean capslock_on = (kbstate&LockMask);
  gboolean is_dayi = !strncmp(cur_inmd->filename, "dayi", 4);

  bzero(seltab_phrase, sizeof(seltab_phrase));

//  dbg("uuuuu %x %x   shift,ctrl:%d,%d\n", key, kbstate, shift_m, ctrl_m);

  if (!cur_inmd)
    return 0;

  if (caps_eng_tog) {
    gboolean new_tsin_pho_mode =!capslock_on;
    if (current_CS->tsin_pho_mode != new_tsin_pho_mode) {
      current_CS->tsin_pho_mode = new_tsin_pho_mode;
      save_CS_current_to_temp();
    }
  }

  if ((kbstate & (Mod1Mask|Mod4Mask|Mod5Mask|ControlMask))==ControlMask
     && key>='1' && key<='9' && ggg.gbufN) {
    save_gtab_buf_phrase(key);
    return 1;
  }

  if (ggg.gbufN && key==XK_Tab)
    return 1;

   if ((key==XK_Shift_L||key==XK_Shift_R) && !key_press_time) {
     key_press_time = current_time();
     key_press_time_ctrl = 0;
   } else
  if ((key==XK_Control_L||key==XK_Control_R) && !key_press_time_ctrl && tss.pre_selN) {
    key_press_time_ctrl = current_time();
    return TRUE;
  } else {
    key_press_time_ctrl = 0;
    key_press_time = 0;
  }

  if (kbstate & (Mod1Mask|Mod4Mask|Mod5Mask|ControlMask)) {
    return 0;
  }

  if (poo.same_pho_query_state == SAME_PHO_QUERY_pho_select)
    return feedkey_pho(key, 0);

  if (poo.same_pho_query_state == SAME_PHO_QUERY_none && gwin_pho &&
    GTK_WIDGET_VISIBLE(gwin_pho))
     hide_win_pho();

  if (!tsin_pho_mode()) {
    if (key < 0x20 || key>=0x7f)
      goto shift_proc;

    if (capslock_on && hime_capslock_lower)
      case_inverse((KeySym *)&key, shift_m);

    if (ggg.gbufN)
      insert_gbuf_cursor_char(key);
    else
      send_ascii(key);

    return 1;
  }


  int lcase;
  lcase = tolower(key);
  int ucase;
  ucase = toupper(key);
  if (key < 127 && cur_inmd->keymap[key]) {
     if (key < 'A' || key > 'z' || key > 'Z'  && key < 'a' )
       goto shift_proc;
     if (cur_inmd->keymap[lcase] != cur_inmd->keymap[ucase])
       goto next;

  }


shift_proc:
  if (shift_m && !strchr(cur_inmd->selkey, key) && !ggg.more_pg && key>=' ' && key < 0x7e &&
       key!='*' && (key!='?' || gtab_shift_phrase_key && !ggg.ci)) {
    dbg("uuuuuuuuuu\n");
    if (gtab_shift_phrase_key) {
      if (tss.pre_selN && shift_char_proc(key, kbstate))
        return TRUE;
      if (feed_phrase(key, kbstate))
        return TRUE;
    } else {
      if (!cur_inmd->keymap[key] || (lcase != ucase &&
           cur_inmd->keymap[lcase]==cur_inmd->keymap[ucase]))
        return shift_char_proc(key, kbstate);
    }
  }

  gboolean has_wild;
  has_wild = FALSE;

  switch (key) {
    case XK_BackSpace:
      ggg.last_idx=0;
      ggg.spc_pressed=0;
      ggg.sel1st_i=MAX_SELKEY-1;
      clear_gtab_input_error_color();
      hide_gtab_pre_sel();

      if (ggg.ci==0) {
        if (AUTO_SELECT_BY_PHRASE)
          return gtab_buf_backspace();
        else
          return 0;
      }

      if (ggg.ci>0)
        ggg.inch[--ggg.ci]=0;

      if (has_wild_card()) {
        proc_wild_disp();
        return 1;
      }


      ggg.wild_mode=0;
      ggg.invalid_spc = FALSE;
      if (ggg.ci==1 && cur_inmd->use_quick) {
        int i;
        clr_seltab();
        for(i=0;i<cur_inmd->M_DUP_SEL;i++)
          utf8cpy(seltab[i], (char *)cur_inmd->qkeys->quick1[ggg.inch[0]-1][i]);

        ggg.defselN=cur_inmd->M_DUP_SEL;
        DispInArea();
        goto Disp_opt;
      } else
      if (ggg.ci==2 && cur_inmd->use_quick) {
        int i;
        clr_seltab();
        for(i=0;i<cur_inmd->M_DUP_SEL;i++)
          utf8cpy(seltab[i], (char *)cur_inmd->qkeys->quick2[ggg.inch[0]-1][ggg.inch[1]-1][i]);

        ggg.defselN=cur_inmd->M_DUP_SEL;
        DispInArea();
        goto Disp_opt;
      }

      break;
    case XK_KP_Enter:
    case XK_Return:
      if (AUTO_SELECT_BY_PHRASE) {
        hide_gtab_pre_sel();
        if (shift_m) {
          return save_gtab_buf_shift_enter();
        } else
          return output_gbuf();
      }
      else
        return 0;
    case XK_Up:
      if (gtab_has_input())
        return TRUE;
      return FALSE;
    case XK_Down:
#if UNIX
    case XK_KP_Down:
#endif

      if (AUTO_SELECT_BY_PHRASE)
        return show_buf_select();
      else
        return 0;
    case XK_Escape:
      hide_gtab_pre_sel();
      if (ggg.gtab_buf_select) {
        ggg.gtab_buf_select = 0;
        reset_gtab_all();
        ClrSelArea();
        if (hime_pop_up_win && !gtab_has_input())
          hide_win_gtab();
        return 1;
      }

      close_gtab_pho_win();
      if (ggg.ci) {
        reset_gtab_all();
        return 1;
      } else {
        if (ggg.gbufN) {
          set_gtab_user_head();
          return 1;
        }
        ClrIn();
        return 0;
      }
    case XK_Prior:
#if UNIX
    case XK_KP_Prior:
#endif
    case XK_KP_Subtract:
      if (ggg.wild_mode) {
        if (ggg.wild_page >= cur_inmd->M_DUP_SEL) ggg.wild_page-=cur_inmd->M_DUP_SEL;
        wildcard();
        return 1;
      } else
      if (ggg.more_pg) {
        if (ggg.gtab_buf_select) {
          gbuf_prev_pg();
          return 1;
        }

        ggg.pg_idx -= page_len();
        if (ggg.pg_idx < ggg.S1)
          ggg.pg_idx = ggg.S1;

        goto next_pg;
      }

      if (key==XK_KP_Subtract)
        goto keypad_proc;

      return win_sym_page_up();
    case XK_Next:
#if UNIX
    case XK_KP_Next:
#endif
    case XK_KP_Add:
      if (ggg.more_pg) {
        if (ggg.gtab_buf_select) {
          gbuf_next_pg();
          return 1;
        }
next_page:
//        dbg("more...\n");
        ggg.pg_idx += page_len();
        if (ggg.pg_idx >=ggg.E1)
          ggg.pg_idx = ggg.S1;
        goto next_pg;
      } else {
        if (key==XK_KP_Add)
          goto keypad_proc;
        if (win_sym_page_down())
          return TRUE;
        if (!ggg.gtab_buf_select && ggg.gbufN && AUTO_SELECT_BY_PHRASE)
          return show_buf_select();
        return FALSE;
      }
    case ' ':
      hide_gtab_pre_sel();

      if (ggg.invalid_spc && gtab_invalid_key_in)
        ClrIn();

      if (!gtab_invalid_key_in && ggg.spc_pressed && ggg.invalid_spc) {
        ClrIn();
        return 1;
      }

      has_wild = has_wild_card();

//      dbg("ggg.wild_mode:%d ggg.more_pg:%d ggg.ci:%d  has_wild:%d\n", ggg.wild_mode, ggg.more_pg, ggg.ci, has_wild);

      if (ggg.wild_mode) {
        // request from tetralet
        if (!ggg.wild_page && ggg.total_matchN < cur_inmd->M_DUP_SEL) {
          ggg.sel1st_i = 0;
          goto direct_select;
        }

        ggg.wild_page += cur_inmd->M_DUP_SEL;
        if (ggg.wild_page >= ggg.total_matchN)
          ggg.wild_page=0;

        wildcard();
        ggg.spc_pressed = TRUE;
        return 1;
      } else
      if (ggg.more_pg && !(_gtab_space_auto_first & GTAB_space_auto_first_any)) {
        if (ggg.gtab_buf_select) {
          gbuf_next_pg();
          return 1;
        }
        else
          goto next_page;
      } else
      if (ggg.ci==0) {
        if (current_CS->b_half_full_char)
          return full_char_proc(key);

        if (ggg.gbufN) {
          output_gbuf();
        } else
          return 0;
      } else
      if (!has_wild) {
//        dbg("iii %d  ggg.defselN:%d   %d\n", ggg.sel1st_i, ggg.defselN, cur_inmd->M_DUP_SEL);
        if (_gtab_space_auto_first == GTAB_space_auto_first_any && seltab[0][0] &&
            ggg.sel1st_i==MAX_SELKEY-1) {
          ggg.sel1st_i = 0;
        }

        if (_gtab_space_auto_first == GTAB_space_auto_first_nofull && ggg.exa_match > 1
            && !AUTO_SELECT_BY_PHRASE && gtab_dup_select_bell)
          bell();

        if (seltab[ggg.sel1st_i][0]) {
//          dbg("ggg.last_full %d %d\n", ggg.last_full,ggg.spc_pressed);
          if (gtab_full_space_auto_first || ggg.spc_pressed) {
direct_select:
            if (AUTO_SELECT_BY_PHRASE && poo.same_pho_query_state != SAME_PHO_QUERY_gtab_input) {
//              dbg("ins ggg.kval %x\n", ggg.kval);
              insert_gbuf_cursor1_cond(seltab[ggg.sel1st_i], ggg.kval, ggg.exa_match);
            }
            else
              putstr_inp(seltab[ggg.sel1st_i]);  /* select 1st */
            return 1;
          }
        }
      }

      ggg.last_full=0;
      ggg.spc_pressed=1;
//      dbg("spc_pressed=1\n");

      if (has_wild) {
        ggg.wild_page=0;
        ggg.wild_mode=1;
        wildcard();
        return 1;
      }

      break;
    case '?':
      if (!gtab_que_wild_card) {
        inkey=cur_inmd->keymap[key];
        if ((inkey && (inkey!=cur_inmd->WILD_QUES && inkey!=cur_inmd->WILD_STAR)) || ptr_selkey(key))
          goto next;
        if (AUTO_SELECT_BY_PHRASE && ggg.gbufN) {
          insert_gbuf_cursor_char(key);
          return 1;
        } else {
          if (current_CS->b_half_full_char)
            return full_char_proc(key);
		  else
            return 0;
		}
      }
    case '*':
      if (tss.pre_selN && shift_char_proc(key, kbstate))
        return TRUE;

      if (current_CS->b_half_full_char)
        return full_char_proc(key);

      inkey=cur_inmd->keymap[key];
      if ((inkey && (inkey!=cur_inmd->WILD_STAR && inkey!=cur_inmd->WILD_QUES)) || ptr_selkey(key)) {
//        dbg("%d %d\n", inkey, cur_inmd->WILD_STAR);
        goto next;
      }
      if (ggg.ci< cur_inmd->MaxPress) {
        ggg.inch[ggg.ci++]=inkey;
        DispInArea();

        if (hime_pop_up_win)
          show_win_gtab();

        ggg.total_matchN = 0;
        ggg.wild_page=0;
        ggg.wild_mode=1;
        wildcard();
        return 1;
      }
      return 0;
    case XK_Left:
#if UNIX
    case XK_KP_Left:
#endif
      return gbuf_cursor_left();
    case XK_Right:
#if UNIX
    case XK_KP_Right:
#endif
      return gbuf_cursor_right();
    case XK_Home:
#if UNIX
    case XK_KP_Home:
#endif
      return gbuf_cursor_home();
    case XK_End:
#if UNIX
    case XK_KP_End:
#endif
      return gbuf_cursor_end();
    case XK_Delete:
#if UNIX
    case XK_KP_Delete:
#endif
      return gtab_buf_delete();
    case XK_Shift_L:
    case XK_Shift_R:
    case XK_Control_R:
    case XK_Control_L:
    case XK_Alt_L:
    case XK_Alt_R:
    case XK_Caps_Lock:
      return 0;
    case '`':
      if (!cur_inmd->keymap[key]) {
        poo.same_pho_query_state = SAME_PHO_QUERY_gtab_input;
        disp_gtab_sel(_(_L("輸入要查的同音字，接著在注音視窗選字")));
        if (hime_pop_up_win)
          show_win_gtab();
        init_gtab_pho_query_win();
        return 1;
      }
    default:
next:

      if (key < 0x7f)
        inkey= cur_inmd->keymap[key];
      else
        inkey = 0;

      if (shift_m && !inkey && !tss.ctrl_pre_sel &&
        tss.pre_selN && shift_char_proc(key, kbstate))
        return TRUE;

      clear_gtab_input_error_color();

      if (ggg.invalid_spc && gtab_invalid_key_in) {
        ClrIn();
      }
      if (key>=XK_KP_0 && key<=XK_KP_9) {
        if (!ggg.ci) {
          if (ggg.gbufN) {
            insert_gbuf_cursor_char(key - XK_KP_0 + '0');
            return 1;
          } else
            return 0;
        }
        if (is_dayi) {
          key = key - XK_KP_0 + '0';
          is_keypad = TRUE;
        }
      }

      int keypad;
keypad_proc:
      keypad = keypad_proc(key);
      if (keypad) {
        if (!ggg.ci) {
          if (ggg.gbufN) {
            insert_gbuf_cursor_char(keypad);
            return 1;
          } else
            return 0;
        }
      }
      char *pendkey = strchr(cur_inmd->endkey, key);

      pselkey=ptr_selkey(key);

      if (!pselkey && (key < 32 || key > 0x7e) && (gtab_full_space_auto_first || ggg.spc_pressed)) {
//        dbg("%x %x ggg.sel1st_i:%d  '%c'\n", pselkey, key, ggg.sel1st_i, seltab[ggg.sel1st_i][0]);
        if (seltab[ggg.sel1st_i][0]) {
          if (AUTO_SELECT_BY_PHRASE && poo.same_pho_query_state != SAME_PHO_QUERY_gtab_input)
            insert_gbuf_cursor1_cond(seltab[ggg.sel1st_i], ggg.kval, ggg.exa_match);
          else
            putstr_inp(seltab[ggg.sel1st_i]);  /* select 1st */
        }

        return 0;
      }



//        dbg("ggg.spc_pressed %d %d %d is_keypad:%d\n", ggg.spc_pressed, ggg.last_full, cur_inmd->MaxPress, is_keypad);

#if 1 // for dayi, testcase :  6 space keypad6
      int vv = pselkey - cur_inmd->selkey;
      if (pselkey && tss.pre_selN && !ggg.gtab_buf_select && (tss.ctrl_pre_sel||
          ((!inkey||ggg.spc_pressed||is_keypad)&&!gtab_disp_partial_match&&!gtab_pre_select))) {
        if (gtab_pre_select_idx(vv))
          return TRUE;
      } else
      if (( (ggg.spc_pressed||ggg.last_full||is_keypad) ||(ggg.wild_mode && (!inkey ||pendkey)) || ggg.gtab_buf_select) && pselkey) {
        if ((_gtab_space_auto_first & GTAB_space_auto_first_any) && !ggg.wild_mode)
          vv++;

        if (vv<0)
          vv=9;

        if (seltab[vv][0]) {
          if (AUTO_SELECT_BY_PHRASE && !same_query_show_pho_win()) {
            if (ggg.gtab_buf_select && poo.same_pho_query_state != SAME_PHO_QUERY_gtab_input)
              set_gbuf_c_sel(vv);
            else
              insert_gbuf_cursor1_cond(seltab[vv], ggg.kval, ggg.exa_match);
          }
          else {
            putstr_inp(seltab[vv]);
          }

          if (hime_pop_up_win && !gtab_has_input())
            hide_win_gtab();

          return 1;
        }
      }
#endif

//      dbg("iii %x sel1st_i:%d\n", pselkey, ggg.sel1st_i);
      if (seltab[ggg.sel1st_i][0] && !ggg.wild_mode &&
           (gtab_full_space_auto_first||ggg.spc_pressed||ggg.last_full) ) {
        if (AUTO_SELECT_BY_PHRASE && poo.same_pho_query_state != SAME_PHO_QUERY_gtab_input)
          insert_gbuf_cursor1_cond(seltab[ggg.sel1st_i], ggg.kval, ggg.exa_match);
        else
          putstr_inp(seltab[ggg.sel1st_i]);  /* select 1st */
      }
#if 0
      if (key > 0x7f) {
        return 0;
      }
#endif

      ggg.spc_pressed=0;

      // for cj & boshiamy to input digits
      if (!ggg.ci && !inkey) {
        if (current_CS->b_half_full_char)
          return full_char_proc(key);
        else {
          if (ggg.gbufN && poo.same_pho_query_state != SAME_PHO_QUERY_gtab_input) {
            insert_gbuf_cursor_char(key);
            return 1;
          }
          else
            return 0;
        }
      }

      if (ggg.wild_mode && inkey>=1 && ggg.ci< cur_inmd->MaxPress) {
        ggg.inch[ggg.ci++]=inkey;
        if (hime_pop_up_win)
          show_win_gtab();
        proc_wild_disp();
        return 1;
      }

      if (inkey>=1 && ggg.ci< cur_inmd->MaxPress) {
        ggg.inch[ggg.ci++]=inkey;
        hide_gtab_pre_sel();

        if (hime_pop_up_win)
          show_win_gtab();
        ggg.last_full=0;

        if (cur_inmd->use_quick && !pendkey) {
          if (ggg.ci==1) {
            int i;
            for(i=0;i < cur_inmd->M_DUP_SEL; i++) {
              utf8cpy(seltab[i], (char *)&cur_inmd->qkeys->quick1[inkey-1][i]);
            }

            ggg.defselN=cur_inmd->M_DUP_SEL;
            DispInArea();
            goto Disp_opt;
          } else
          if (ggg.ci==2 && !pselkey) {
            int i;
            for(i=0;i < cur_inmd->M_DUP_SEL; i++) {
              utf8cpy(seltab[i], (char *)&cur_inmd->qkeys->quick2[ggg.inch[0]-1][inkey-1][i]);
            }

            ggg.defselN=cur_inmd->M_DUP_SEL;
            DispInArea();
            goto Disp_opt;
          }
        }
      } else
      if (ggg.ci == cur_inmd->MaxPress && !pselkey) {
        bell();
        return 1;
      }


      if (inkey) {
        for(i=0; i < MAX_TAB_KEY_NUM64_6; i++)
          if (ggg.inch[i]>=cur_inmd->WILD_QUES) {
            DispInArea();
            if (ggg.ci==cur_inmd->MaxPress) {
              ggg.wild_mode=1;
              ggg.wild_page=0;
              wildcard();
            }

            return 1;
          }
      } else {
        if (!pselkey) {
          if (current_CS->b_half_full_char)
            return full_char_proc(key);
          else {
            if (key>=' ' && key<0x7f && AUTO_SELECT_BY_PHRASE && ggg.gbufN)
              insert_gbuf_cursor_char(key);
            else
              return 0;
          }
        }

        if (ggg.defselN) {
          goto YYYY;
        }
     }
  } /* switch */


  if (ggg.ci==0) {
    ClrSelArea();
    ClrIn();
    return 1;
  }

  ggg.invalid_spc = FALSE;
  char *pendkey = NULL;
  pendkey = strchr(cur_inmd->endkey, key);

  DispInArea();

  ggg.kval=0;

  for(i=0; i < Max_tab_key_num; i++) {
    ggg.kval|= (u_int64_t)ggg.inch[i] << (KeyBits * (Max_tab_key_num - 1 - i));
  }

#if 1
  if (ggg.last_idx)
    ggg.S1=ggg.last_idx;
  else
#endif
    ggg.S1=cur_inmd->idx1[ggg.inch[0]];

//  dbg("--------- ch:%d %d val %llx  ggg.S1:%d\n", ggg.inch[0], Max_tab_key_num, ggg.kval, ggg.S1);

  int oE1;
  oE1=cur_inmd->idx1[ggg.inch[0]+1];
  if (cur_inmd->keybits==6)
    vmaskci = cur_inmd->key64 ? vmask64[ggg.ci]:vmask[ggg.ci];
  else
    vmaskci = cur_inmd->key64 ? vmask64_7[ggg.ci]:vmask_7[ggg.ci];

  gtab_scan_pre_select(TRUE);

  while ((CONVT2(cur_inmd, ggg.S1) & vmaskci) != ggg.kval &&
          CONVT2(cur_inmd, ggg.S1) < ggg.kval &&  ggg.S1<oE1)
    ggg.S1++;

  ggg.pg_idx=ggg.last_idx=ggg.S1;


#if 0
  dbg("MaxPress:%d vmaskci:%llx kval:%llx ggg.ci:%d  !=%d  S1:%d  kval:%x\n", cur_inmd->MaxPress,
  vmaskci, ggg.kval, ggg.ci,
  ((CONVT2(cur_inmd, ggg.S1) & vmaskci)!=ggg.kval), ggg.S1);
#endif

  if ((CONVT2(cur_inmd, ggg.S1) & vmaskci)!=ggg.kval || (ggg.wild_mode && ggg.defselN) ||
                  ((/* ggg.ci==cur_inmd->MaxPress|| */ ggg.spc_pressed) && ggg.defselN &&
      (pselkey && ( pendkey || ggg.spc_pressed)) ) ) {
YYYY:

    if ((pselkey || ggg.wild_mode) && ggg.defselN) {
      int vv = pselkey - cur_inmd->selkey;

      if ((_gtab_space_auto_first & GTAB_space_auto_first_any) && !ggg.wild_mode
          && ggg.exa_match && (!cur_inmd->use_quick || ggg.ci!=2))
        vv++;

      if (vv<0)
        vv=9;

      if (seltab[vv][0]) {
        if (AUTO_SELECT_BY_PHRASE && poo.same_pho_query_state != SAME_PHO_QUERY_gtab_input)
          insert_gbuf_cursor1_cond(seltab[vv], ggg.kval, ggg.exa_match);
        else
          putstr_inp(seltab[vv]);
        return 1;
      }
    }

    if (pselkey && !ggg.defselN)
      return 0;

    if (gtab_invalid_key_in) {
      if (ggg.spc_pressed) {
        bell_err();
        ggg.invalid_spc = TRUE;
//        dbg("ggg.invalid_spc\n");
      } else {
        seltab[0][0]=0;
        ClrSelArea();
      }
    } else {
      if (gtab_dup_select_bell)
        bell();

      if (ggg.ci>0)
        ggg.inch[--ggg.ci]=0;
    }

    ggg.last_idx=0;
    DispInArea();
    return 1;
  }

//refill:

  j=ggg.S1;
  while(CONVT2(cur_inmd, j)==ggg.kval && j<oE1)
    j++;

  ggg.E1 = j;
  ggg.total_matchN = ggg.E1 - ggg.S1;
  ggg.pg_idx = ggg.S1;

  ggg.more_pg = 0;
  if (ggg.total_matchN > page_len()) {
    if ((_gtab_space_auto_first & GTAB_space_auto_first_any) || ggg.spc_pressed || pendkey ||
      ggg.ci==cur_inmd->MaxPress && (_gtab_space_auto_first & GTAB_space_auto_first_full))
      ggg.more_pg = 1;
  }

  if (ggg.ci < cur_inmd->MaxPress && !ggg.spc_pressed && !pendkey && !ggg.more_pg) {
    j = ggg.S1;
    ggg.exa_match=0;
    clr_seltab();
    int match_cnt=0;

    while (CONVT2(cur_inmd, j)==ggg.kval && ggg.exa_match <= page_len()) {
      seltab_phrase[ggg.exa_match] = load_seltab(j, ggg.exa_match);
      match_cnt++;
      ggg.exa_match++;
      j++;
    }

    ggg.defselN=ggg.exa_match;
//    dbg("--- ggg.exa_match %d\n", ggg.exa_match);

    if (ggg.defselN > page_len())
      ggg.defselN--;

    int shiftb=(KEY_N - 1 -ggg.ci) * KeyBits;

//    if (gtab_disp_partial_match)
    while((CONVT2(cur_inmd, j) & vmaskci)==ggg.kval && j<oE1) {
      int fff=cur_inmd->keycol[(CONVT2(cur_inmd, j)>>shiftb) & cur_inmd->kmask];
      u_char *tbl_ch = tblch(j);

      if (disp_partial_match_on() && (!seltab[fff][0] || seltab_phrase[fff] ||
           (bchcmp(seltab[fff], tbl_ch)>0 && fff > ggg.exa_match))) {
        seltab_phrase[fff] = load_seltab(j, fff);
        ggg.defselN++;
      }

      match_cnt++;
#if 0
      dbg("jj %d", fff); utf8_putchar(seltab[fff]); dbg("\n");
#endif
      j++;
    }

    if (gtab_unique_auto_send) {
      char *first_str=NULL;
      for(i=0; i < page_len(); i++) {
        if (!seltab[i][0])
          continue;
        if (!first_str)
          first_str = seltab[i];
      }

      if (match_cnt==1 && first_str) {
        if (AUTO_SELECT_BY_PHRASE && poo.same_pho_query_state != SAME_PHO_QUERY_gtab_input)
          insert_gbuf_nokey(first_str);
        else
          putstr_inp(first_str);
        return 1;
      }
    }
  } else {
//    dbg("more %d %d  skip_end:%d\n", ggg.more_pg,  ggg.total_matchN, cur_inmd->flag&FLAG_PHRASE_AUTO_SKIP_ENDKEY);
next_pg:
    ggg.defselN=0;
    clr_seltab();
    if (pendkey && (!(cur_inmd->flag&FLAG_PHRASE_AUTO_SKIP_ENDKEY) || !AUTO_SELECT_BY_PHRASE || ggg.ci==1)) {
//      dbg("spc_pressed = 1\n");
      ggg.spc_pressed = 1;
    }

    if (ggg.ci==cur_inmd->MaxPress)
      ggg.last_full=1;
    int full_send = gtab_press_full_auto_send && ggg.last_full;

//    dbg("flag %d\n",!(pendkey && (cur_inmd->flag&FLAG_PHRASE_AUTO_SKIP_ENDKEY)));
    if (AUTO_SELECT_BY_PHRASE && !(pendkey && (cur_inmd->flag&FLAG_PHRASE_AUTO_SKIP_ENDKEY))
        && poo.same_pho_query_state != SAME_PHO_QUERY_gtab_input &&
        (ggg.spc_pressed||full_send)) {
      j = ggg.S1;
      int selN=0;
      char **sel = NULL;

//     puts("kkkkkkkkkkk");
      while(j<ggg.E1 && CONVT2(cur_inmd, j)==ggg.kval && selN < 255) {
        sel = trealloc(sel, char *, selN+1);
        sel[selN++] = load_tblidx(j);
        j++;
      }
      insert_gbuf_cursor(sel, selN, ggg.kval, FALSE);
      gtab_scan_pre_select(FALSE);
      clear_after_put();
      return 1;
    } else {
      j = ggg.pg_idx;

//      dbg("jjjjjjjjjjjjjjjjjj");
      while(j<ggg.E1 && CONVT2(cur_inmd, j)==ggg.kval && ggg.defselN < page_len()) {
        load_seltab(j, ggg.defselN);

        j++; ggg.defselN++;

        if (ggg.ci == cur_inmd->MaxPress || ggg.spc_pressed) {
          ggg.sel1st_i=0;
//          dbg("ggg.sel1st_i %d %d %d\n", ggg.ci, cur_inmd->MaxPress, ggg.spc_pressed);
        }
      }
    }

    ggg.exa_match = ggg.defselN;
//    dbg("ggg.defselN %d\n", ggg.defselN);


    if (ggg.defselN==1 && !ggg.more_pg) {
      if (ggg.spc_pressed || full_send || gtab_unique_auto_send) {
        if (AUTO_SELECT_BY_PHRASE && poo.same_pho_query_state != SAME_PHO_QUERY_gtab_input)
          insert_gbuf_cursor1_cond(seltab[0], ggg.kval, ggg.exa_match);
        else
          putstr_inp(seltab[0]);
        return 1;
      }
    } else
    if (!ggg.defselN) {
      bell_err();
//      ggg.spc_pressed=0;
//      if (gtab_invalid_key_in)
      {
        ggg.invalid_spc = TRUE;
        return TRUE;
      }

      return TRUE;
    } else
    if (!ggg.more_pg) {
      if (gtab_dup_select_bell && (disp_partial_match_on() || gtab_pre_select_on())) {
        if (ggg.spc_pressed || gtab_full_space_auto_first || ggg.last_full && gtab_press_full_auto_send)
          bell();
      }
    }
  }

Disp_opt:
  if (disp_partial_match_on() || gtab_pre_select_on() || ((ggg.exa_match > 1 || ggg.more_pg) &&
    (ggg.spc_pressed || gtab_press_full_auto_send ||
    (ggg.ci==cur_inmd->MaxPress && (_gtab_space_auto_first & GTAB_space_auto_first_full))) ) ) {
       disp_selection(phrase_selected);
  }

  return 1;
}

#if WIN32
static GTAB_ST temp_st;
void pho_save_gst(), pho_restore_gst();
// static GEDIT *gbuf_save;
static char **seltab_save;

void gtab_save_gst()
{
  init_seltab(&seltab_save);
  int i;
  for(i=0; i < MAX_SELKEY; i++)
    strcpy(seltab_save[i], seltab[i]);

  pho_save_gst();
  temp_st = ggg;
}

void gtab_restore_gst()
{
  int i;
  for(i=0; i < MAX_SELKEY; i++)
    strcpy(seltab[i], seltab_save[i]);

  pho_restore_gst();
  ggg = temp_st;
}
#endif
