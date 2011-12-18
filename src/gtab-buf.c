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
#include "gtab.h"
#include "hime-conf.h"
#include "hime-endian.h"
#include "pho.h"
#include "tsin.h"
#include "tsin-parse.h"
#include "win-save-phrase.h"
#include "gtab-buf.h"
#include "gst.h"
#if WIN32
#include <io.h>
#endif

void disp_gbuf(), ClrIn(), clear_after_put();
gboolean gtab_phrase_on();
int page_len();
void show_win_gtab();
void disp_selection0(gboolean phrase_selected, gboolean force_disp);
void disp_gtab_sel(char *s);
void add_cache(int start, int usecount, TSIN_PARSE *out, short match_phr_N, short no_match_ch_N, int tc_len);
int ch_pos_find(char *ch, int pos);
void inc_gtab_usecount(char *str), ClrSelArea();
void lookup_gtabn(char *ch, char *out);
char *htmlspecialchars(char *s, char out[]);
void hide_gtab_pre_sel();

extern gint64 key_press_time, key_press_time_ctrl;

extern gboolean test_mode;

GEDIT *gbuf;
extern char **seltab;
extern int ph_key_sz;

void extract_gtab_key(int start, int len, void *out)
{
  int i;

  char *p=(char *)out;
  if (ph_key_sz==4) {
    for(i=0; i < len; i++) {
      u_int k = gbuf[i+start].keys[0];
      memcpy(p, &k, sizeof(k));
      p+=sizeof(k);
    }
  } else {
    for(i=0; i < len; i++) {
      memcpy(p, &gbuf[i+start].keys[0], sizeof(u_int64_t));
      p+=sizeof(u_int64_t);
    }
  }
}

void extract_gbuf_str(int start, int len, char *out)
{
  int i;
  out[0]=0;
  for(i=0;i<len;i++)
    strcat(out, gbuf[i+start].ch);
}


gboolean gtab_cursor_end()
{
  return ggg.gbuf_cursor==ggg.gbufN;
}

void dump_gbuf()
{
  int i;

  for(i=0; i<ggg.gbufN; i++) {
    int j;
    for(j=0;j < gbuf[i].selN; j++)
      printf("%d:%s ", j, gbuf[i].sel[j]);
    puts("");
  }
}

static unich_t latin_chars[]=
_L("ÀÁÂÃÄÅÆÆÇÈÉÊËÌÍÎÏÐÐÑÒÓÔÕÖØÙÚÛÜÝÞÞßàáâãäåææçèéêëìíîïððñòóôõöøùúûüýþþÿ")
_L("ĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħĨĩĪīĬĭĮįİıĲĲĳĳĴĵĶķĸĹĺĻļĽľĿŀŁł")
_L("ŃńŅņŇňŉŊŋŌōŎŏŐőŒŒœœŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽž");

int en_word_len(char *bf)
{
  char *s;

  for(s=bf;*s;) {
    int sz = utf8_sz(s);
    if (sz==1) {
      if (!(*s >= 'A' && *s<='Z' || *s >= 'a' && *s<='z' || strchr("-_'", *s)))
        break;
    } else
    if (sz==2) {
      char *p;
#if WIN32
      p = _(latin_chars);
      for (; *p; p+=2)
#else
      for (p=latin_chars; *p; p+=2)
#endif
        if (!memcmp(p, s, 2))
          break;
      if (!(*p))
        break;
    } else
    if (sz>=3)
      break;
    s+=sz;
  }

  if (*s)
    return 0;
  return strlen(bf);
}

static char *gen_buf_str(int start, gboolean add_spc)
{
  int i;
  char *out = tmalloc(char, 1);
  int outN=0;

  gboolean last_en_word = FALSE;
  for(i=start;i<ggg.gbufN;i++) {
    char *t = gbuf[i].ch;
    int len = strlen(t);

    if (add_spc && en_word_len(t) && !(gbuf[i].flag & FLAG_CHPHO_GTAB_BUF_EN_NO_SPC)) {
      if (last_en_word) {
        out = trealloc(out, char, outN+1);
        out[outN++]=' ';
      }
      last_en_word = TRUE;
    } else
      last_en_word = FALSE;

    out = trealloc(out, char, outN+len+1);
    memcpy(out + outN, t, len);
    outN+=len;
  }

  out[outN] = 0;
  return out;
}

extern gboolean last_cursor_off;

static char *gen_buf_str_disp()
{
  if (!ggg.gbufN) {
    return strdup("");
  }

  int i;
  char *out = tmalloc(char, 1);
  int outN=0;

  out[0]=0;
  gbuf[ggg.gbufN].ch = " ";

  gboolean last_is_en_word = FALSE;

  int N = last_cursor_off ? ggg.gbufN-1:ggg.gbufN;
  for(i=0;i<=N;i++) {
    char addspc[MAX_CIN_PHR * 2 + 2];
    char spec[MAX_CIN_PHR * 2 + 2];
    int len = en_word_len(gbuf[i].ch);
//    dbg("i %d N:%d bufN:%d\n",i,N,ggg.gbufN);
    if (gbuf[i].flag & FLAG_CHPHO_GTAB_BUF_EN_NO_SPC)
      len = 0;
//    dbg("%d %d is_en:%d\n",i, len, last_is_en_word);

    if (len) {
      if (last_is_en_word) {
        strcpy(addspc, " ");
        strcat(addspc, gbuf[i].ch);
      } else
        strcpy(addspc, gbuf[i].ch);
      last_is_en_word = TRUE;
    } else {
      last_is_en_word = FALSE;
      strcpy(addspc, gbuf[i].ch);
    }

    htmlspecialchars(addspc, spec);
//    dbg("addspc '%s'  spec:%s out:%s\n", addspc, spec, out);

    char www[MAX_CIN_PHR * 2 + 2];
    char *t = spec;

    if (i==ggg.gbuf_cursor) {
      sprintf(www, "<span background=\"%s\">%s</span>", tsin_cursor_color, spec);
      t = www;
    }

    len = strlen(t);
    out = trealloc(out, char, outN+len+1);
    memcpy(out + outN, t, len);
    outN+=len;
    out[outN] = 0;
  }

  return out;
}


void disp_label_edit(char *str);

static void free_pgbuf(GEDIT *p)
{
  int i;
  for(i=0; i < p->selN; i++)
    free(p->sel[i]);
  free(p->sel);
  p->ch = NULL;
  p->sel=NULL;
  p->flag = 0;
}


static void free_gbuf(int idx)
{
  free_pgbuf(&gbuf[idx]);
}


static void clear_gtab_buf_all()
{
  int i;
  for(i=0;i<ggg.gbufN;i++)
    free_gbuf(i);
  ggg.gbuf_cursor = ggg.gbufN=0;
  ggg.gtab_buf_select = 0;
  disp_gbuf();
}


void minimize_win_gtab();
void disp_gbuf()
{
#if WIN32
  if (test_mode)
    return;
#endif
  char *bf=gen_buf_str_disp();
  disp_label_edit(bf);

  if (ggg.gbufN && gtab_disp_key_codes)
    lookup_gtabn(gbuf[ggg.gbufN-1].ch, NULL);

  free(bf);

  minimize_win_gtab();
}

void clear_gbuf_sel()
{
  ggg.gtab_buf_select = 0;
  ggg.total_matchN = 0;
  ClrSelArea();
}

int gbuf_cursor_left()
{
  hide_gtab_pre_sel();
  if (!ggg.gbuf_cursor)
    return ggg.gbufN;
#if WIN32
  if (test_mode)
    return 1;
#endif
  if (ggg.gtab_buf_select)
    clear_gbuf_sel();
  ClrIn();
  ggg.gbuf_cursor--;
  disp_gbuf();
  return 1;
}


int gbuf_cursor_right()
{
  hide_gtab_pre_sel();
  if (ggg.gbuf_cursor==ggg.gbufN)
    return ggg.gbufN;
#if WIN32
  if (test_mode)
    return 1;
#endif
  if (ggg.gtab_buf_select)
    clear_gbuf_sel();
  ggg.gbuf_cursor++;
  disp_gbuf();
  return 1;
}

int gbuf_cursor_home()
{
  hide_gtab_pre_sel();
  if (!ggg.gbufN)
    return 0;
#if WIN32
  if (test_mode)
    return 1;
#endif
  if (ggg.gtab_buf_select)
    clear_gbuf_sel();

  ggg.gbuf_cursor = 0;
  disp_gbuf();
  return 1;
}


int gbuf_cursor_end()
{
  hide_gtab_pre_sel();
  if (!ggg.gbufN)
    return 0;
#if WIN32
  if (test_mode)
    return 1;
#endif
  if (ggg.gtab_buf_select)
    clear_gbuf_sel();

  ggg.gbuf_cursor = ggg.gbufN;
  disp_gbuf();
  return 1;
}

void inc_gtab_use_count(char *s);
void inc_dec_tsin_use_count(void *pho, char *ch, int N);

gboolean output_gbuf()
{
  hide_gtab_pre_sel();

  if (!ggg.gbufN)
    return FALSE;
#if WIN32
  if (test_mode)
    return TRUE;
#endif
  char *bf=gen_buf_str(0, TRUE);
#if 0
  printf("out %s\n", bf);
#endif

#if 0
  // single character
  char *p;
  for(p=bf; *p; p+=utf8_sz(p))
    inc_gtab_use_count(p);
#endif

  send_text(bf);
  free(bf);

  int i;
  for(i=0; i < ggg.gbufN;) {
    char t[MAX_CIN_PHR+1];
    t[0]=0;
    inc_gtab_use_count(gbuf[i].ch);

    int j;
    for(j=i; j < i+gbuf[i].plen; j++)
      strcat(t, gbuf[j].ch);

    if (!gbuf[i].plen)
      i++;
#if USE_TSIN
    else {
      u_int64_t kk[MAX_PHRASE_LEN];
	  extract_gtab_key(i, gbuf[i].plen, kk);
	  inc_dec_tsin_use_count(kk, t, gbuf[i].plen);
      i+=gbuf[i].plen;
    }
#endif
  }


  clear_gtab_buf_all();
  ClrIn();
  return TRUE;
}


gboolean check_gtab_fixed_mismatch(int idx, char *mtch, int plen)
{
  int j;
  char *p = mtch;

  for(j=0; j < plen; j++) {
    int u8sz = utf8_sz(p);
    if (!(gbuf[idx+j].flag & FLAG_CHPHO_FIXED))
      continue;

    if (memcmp(gbuf[idx+j].ch, p, u8sz))
      break;

    p+= u8sz;
  }

  if (j < plen)
    return TRUE;

  return FALSE;
}

void set_gtab_user_head()
{
  gbuf[ggg.gbuf_cursor].flag |= FLAG_CHPHO_PHRASE_USER_HEAD;
}


CACHE *cache_lookup(int start);

#define DBG 0

void init_cache();
void free_cache();
void init_tsin_table();
void set_tsin_parse_len(int);

void gtab_parse()
{
  int i;
  TSIN_PARSE out[MAX_PH_BF_EXT+1];
  bzero(out, sizeof(out));

  if (test_mode)
    return;

  if (ggg.gbufN <= 1)
    return;

  init_tsin_table();

  init_cache(ggg.gbufN);

  set_tsin_parse_len(ggg.gbufN);

  short smatch_phr_N, sno_match_ch_N;
  tsin_parse_recur(0, out, &smatch_phr_N, &sno_match_ch_N);
#if 0
  puts("vvvvvvvvvvvvvvvv");
  for(i=0;  i < out[i].len; i++) {
    printf("%x %d:", out[i].str, out[i].len);
    utf8_putcharn(out[i].str, out[i].len);
  }
  dbg("\n");
#endif

  for(i=0; i < ggg.gbufN; i++)
    gbuf[i].flag &= ~(FLAG_CHPHO_PHRASE_HEAD|FLAG_CHPHO_PHRASE_BODY);

  int ofsi;
  for(ofsi=i=0; out[i].len; i++) {
    int j, ofsj;

    if (out[i].flag & FLAG_TSIN_PARSE_PHRASE) {
      gbuf[ofsi].flag |= FLAG_CHPHO_PHRASE_HEAD;
      gbuf[ofsi].plen = out[i].len;
    }

    for(ofsj=j=0; j < out[i].len; j++) {
      char *w = (char *)&out[i].str[ofsj];
      int wsz = utf8_sz(w);
      ofsj += wsz;

      int k;
      for(k=0;k<gbuf[ofsi].selN; k++) {
        int sz = utf8_sz(gbuf[ofsi].sel[k]);
        if (wsz == sz && !memcmp(gbuf[ofsi].sel[k], w, sz))
          break;
      }
      if (k==gbuf[ofsi].selN) {
#if 0
        dbg("qq ");
        utf8_putchar(w);
        p_err(" err 1 selN:%d ofsi:%d", gbuf[ofsi].selN, ofsi);
#endif
        k=0;
      }

      if (!(gbuf[ofsi].flag & FLAG_CHPHO_FIXED)) {
        gbuf[ofsi].ch = gbuf[ofsi].sel[k];
        gbuf[ofsi].c_sel = k;
      }
      gbuf[ofsi].flag |= FLAG_CHPHO_PHRASE_BODY;

      ofsi++;
    }
  }

#if 0
  puts("-----------------------------");
  for(i=0;i<ggg.gbufN;i++)
    puts(gbuf[i].ch);
#endif
  free_cache();
}

static GEDIT *cursor_gbuf()
{
  return ggg.gbuf_cursor == ggg.gbufN ? &gbuf[ggg.gbuf_cursor-1] : &gbuf[ggg.gbuf_cursor];
}

typedef struct {
  char *s;
  int usecount;
  int org_seq;
} GITEM;

int get_gtab_use_count(char *s);

int qcmp_gitem(const void *aa, const void *bb)
{
  int d = ((GITEM *)bb)->usecount - ((GITEM *)aa)->usecount;
  if (d)
    return d;

  return ((GITEM *)aa)->org_seq - ((GITEM *)bb)->org_seq;
}

void hide_row2_if_necessary();

unich_t auto_end_punch[]=_L(", . ? : ; ! [ ] 「 」 ， 。 ？ ； ： 、 ～ ！ （ ）");
GEDIT *insert_gbuf_cursor(char **sel, int selN, u_int64_t key, gboolean b_gtab_en_no_spc)
{
  hide_row2_if_necessary();

  if (!sel || !selN)
    return NULL;
//  dbg("insert_gbuf_cursor %x\n", key);

  gbuf=trealloc(gbuf, GEDIT, ggg.gbufN+2);

  GEDIT *pbuf = &gbuf[ggg.gbuf_cursor];

  if (ggg.gbuf_cursor < ggg.gbufN)
    memmove(&gbuf[ggg.gbuf_cursor+1], &gbuf[ggg.gbuf_cursor], sizeof(GEDIT) * (ggg.gbufN - ggg.gbuf_cursor));

  ggg.gbuf_cursor++;
  ggg.gbufN++;

  bzero(pbuf, sizeof(GEDIT));
  bzero(gbuf+ggg.gbufN, sizeof(GEDIT));

  GITEM *items = tmalloc(GITEM, selN);

  int i;
  for(i=0; i < selN; i++) {
    items[i].s = sel[i];
    items[i].org_seq = i;
    items[i].usecount = get_gtab_use_count(sel[i]);
  }
  qsort(items, selN, sizeof(GITEM), qcmp_gitem);

  for(i=0; i < selN; i++)
    sel[i] = items[i].s;

  pbuf->ch = sel[0];
  pbuf->sel = sel;
  pbuf->selN = selN;
  pbuf->c_sel = 0;
  pbuf->keys[0] = key;
  pbuf->keysN=1;
  pbuf->flag = b_gtab_en_no_spc ? FLAG_CHPHO_GTAB_BUF_EN_NO_SPC:0;

  if (ggg.gbufN==ggg.gbuf_cursor && selN==1 && strstr(_(auto_end_punch), sel[0])) {
    char_play(pbuf->ch);
    output_gbuf();
  } else {
    gtab_parse();
    disp_gbuf();
    char_play(pbuf->ch);
  }

  free(items);
  return pbuf;
}


void set_gbuf_c_sel(int v)
{
  GEDIT *pbuf = cursor_gbuf();

  pbuf->c_sel = v + ggg.pg_idx;
  pbuf->ch = pbuf->sel[pbuf->c_sel];
//  dbg("zzzsel v:%d %d %s\n",v, pbuf->c_sel,pbuf->ch);
  pbuf->flag |= FLAG_CHPHO_FIXED;
  ggg.gtab_buf_select = 0;
  ggg.more_pg = 0;
  disp_gtab_sel("");
  gtab_parse();
  disp_gbuf();
//  dbg("zzzsel v:%d\n", pbuf->c_sel);
}

GEDIT *insert_gbuf_cursor1(char *s, u_int64_t key, gboolean b_gtab_en_no_spc)
{
   if (!gtab_phrase_on())
     return NULL;

//   dbg("insert_gbuf_cursor1 %s %x\n", s, key);
   char **sel = tmalloc(char *, 1);
   sel[0] = strdup(s);
   GEDIT *e = insert_gbuf_cursor(sel, 1, key, b_gtab_en_no_spc);
   clear_after_put();
   return e;
}

void insert_gbuf_cursor_phrase(char *s, void *key, int N)
{
  u_int *key32 = (u_int *)key;
  u_int64_t *key64 = (u_int64_t *)key;

  int i;
  for(i=0; i < N; i++) {
    char ch[CH_SZ+1];
    int n = utf8cpy(ch, s);
    u_int64_t v = ph_key_sz==4?key32[i]:key64[i];
    GEDIT *e = insert_gbuf_cursor1(ch, v, TRUE);
    e->flag |= FLAG_CHPHO_FIXED;
    s+=n;
  }
}

static int key_N(u_int64_t k)
{
  int n=0;
  int mask = (1 << KeyBits) - 1;

  while (k) {
    k>>=mask;
    n++;
  }

  return n;
}

static int qcmp_key_N(const void *aa, const void *bb)
{
  u_int64_t a = *((u_int64_t *)aa);
  u_int64_t b = *((u_int64_t *)bb);

  return key_N(a) - key_N(b);
}


void insert_gbuf_nokey(char *s)
{
#if WIN32
   if (test_mode)
     return;
#endif
   if (!gtab_phrase_on())
     return;

//   dbg("insert_gbuf_nokey\n");

   int i;
   u_int64_t keys[32];
   int keysN=0;
   int sz = utf8_sz(s);

   keys[0]=0;
   if (cur_inmd->tbl64) {
     for(i=0; i < cur_inmd->DefChars; i++) {
       if (!memcmp(cur_inmd->tbl64[i].ch, s, sz)) {
         u_int64_t t;
         memcpy(&t, cur_inmd->tbl64[i].key, sizeof(u_int64_t));
         keys[keysN++] = t;
       }
     }
   } else
   if (cur_inmd->tbl) {
     for(i=0; i < cur_inmd->DefChars; i++) {
       if (!memcmp(cur_inmd->tbl[i].ch, s, sz)) {
         u_int t;
         memcpy(&t, cur_inmd->tbl[i].key, sizeof(u_int));
         keys[keysN++] = t;
       }
     }
   }

   qsort(keys, keysN, sizeof(u_int64_t), qcmp_key_N);

   GEDIT *e = insert_gbuf_cursor1(s, keys[0], TRUE);
   if (keysN > 8)
     keysN = 8;

   memcpy(e->keys, keys, sizeof(u_int64_t) * keysN);
   e->keysN = keysN;
}

void insert_gbuf_cursor1_cond(char *s, u_int64_t key, gboolean valid_key)
{
#if WIN32
  if (test_mode)
    return;
#endif

  if (valid_key)
    insert_gbuf_cursor1(s, key, FALSE);
  else
    insert_gbuf_nokey(s);
}

void insert_gbuf_cursor_char(char ch)
{
#if WIN32
  if (test_mode)
    return;
#endif
  char t[2];
  t[0]=ch;
  t[1]=0;
  insert_gbuf_cursor1(t, 0, TRUE);
}

gboolean gtab_has_input();
void hide_win_gtab();

int gtab_buf_delete_ex(gboolean auto_hide)
{
  if (ggg.gbuf_cursor==ggg.gbufN)
    return 0;

  if (test_mode)
    return 1;

  if (ggg.gtab_buf_select)
    clear_gbuf_sel();

  free_gbuf(ggg.gbuf_cursor);
  memmove(&gbuf[ggg.gbuf_cursor], &gbuf[ggg.gbuf_cursor+1], sizeof(GEDIT) * (ggg.gbufN - ggg.gbuf_cursor -1));
  ggg.gbufN--;
  disp_gbuf();

  if (hime_pop_up_win && !gtab_has_input() && auto_hide)
    hide_win_gtab();

  return 1;
}

int gtab_buf_delete()
{
  return gtab_buf_delete_ex(TRUE);
}


gboolean gtab_has_input();
void hide_win_gtab();

int gtab_buf_backspace_ex(gboolean auto_hide)
{
  if (!ggg.gbuf_cursor) {
    return ggg.gbufN>0;
  }

#if WIN32
  if (test_mode)
    return 1;
#endif

  ggg.gbuf_cursor--;
  gtab_buf_delete_ex(auto_hide);

  if (hime_pop_up_win && !gtab_has_input() && auto_hide)
    hide_win_gtab();

  return 1;
}

int gtab_buf_backspace()
{
  return gtab_buf_backspace_ex(TRUE);
}


void gtab_buf_backspaceN(int n)
{
  int i;
  for(i=0; i < n; i++)
    gtab_buf_backspace_ex(FALSE);
}

extern int more_pg;

void gtab_disp_sel()
{
  int idx = ggg.gbuf_cursor==ggg.gbufN ? ggg.gbuf_cursor-1:ggg.gbuf_cursor;
  GEDIT *pbuf=&gbuf[idx];

  int i;
  for(i=0; i < cur_inmd->M_DUP_SEL; i++) {
    int v = i + ggg.pg_idx;
    if (v >= pbuf->selN)
      seltab[i][0]=0;
    else
      strcpy(seltab[i], pbuf->sel[v]);
  }

  if (pbuf->selN > page_len())
    ggg.more_pg = 1;
#if WIN32
  show_win_gtab();
  disp_selection0(FALSE, TRUE);
#else
  disp_selection0(FALSE, TRUE);
  show_win_gtab();
#endif
}


int show_buf_select()
{
  if (!ggg.gbufN)
    return 0;

  int idx = ggg.gbuf_cursor==ggg.gbufN ? ggg.gbuf_cursor-1:ggg.gbuf_cursor;
  GEDIT *pbuf=&gbuf[idx];
  ggg.gtab_buf_select = 1;
  ggg.total_matchN = pbuf->selN;
  ggg.pg_idx = 0;

  gtab_disp_sel();
  hide_gtab_pre_sel();

  return 1;
}

void gbuf_prev_pg()
{
  ggg.pg_idx -= page_len();
  if (ggg.pg_idx < 0)
    ggg.pg_idx = 0;

  gtab_disp_sel();
}

void gbuf_next_pg()
{
  ggg.pg_idx += page_len();
  if (ggg.pg_idx >= ggg.total_matchN)
    ggg.pg_idx = 0;

  gtab_disp_sel();
}

#include "im-client/hime-im-client-attr.h"

int get_DispInArea_str(char *out);

int gtab_get_preedit(char *str, HIME_PREEDIT_ATTR attr[], int *pcursor, int *sub_comp_len)
{
  int i=0;
  int strN=0;
  int attrN=0;
  int ch_N=0;

//  dbg("gtab_get_preedit\n");
  str[0]=0;
  *pcursor=0;

#if WIN32 || 1
  *sub_comp_len = ggg.ci > 0;
#if 1
  if (ggg.gbufN && !hime_edit_display_ap_only())
	*sub_comp_len|=4;
#endif
#endif
  gboolean ap_only = hime_edit_display_ap_only();

  if (gtab_phrase_on()) {
    attr[0].flag=HIME_PREEDIT_ATTR_FLAG_UNDERLINE;
    attr[0].ofs0=0;

    if (ggg.gbufN)
      attrN=1;

    gboolean last_is_en_word = FALSE;
    for(i=0; i < ggg.gbufN; i++) {
      char *s = gbuf[i].ch;
      char tt[MAX_CIN_PHR+2];

      if (en_word_len(s) && !(gbuf[i].flag & FLAG_CHPHO_GTAB_BUF_EN_NO_SPC)) {
        if (last_is_en_word) {
          strcpy(tt, " ");
          strcat(tt, s);
          s = tt;
        }
        last_is_en_word = TRUE;
      } else {
        last_is_en_word = FALSE;
      }

      int len = strlen(s);
      int N = utf8_str_N(s);
      ch_N+=N;
      if (i < ggg.gbuf_cursor)
        *pcursor+=N;
      if (ap_only && i==ggg.gbuf_cursor) {
        attr[1].ofs0=*pcursor;
        attr[1].ofs1=*pcursor+N;
        attr[1].flag=HIME_PREEDIT_ATTR_FLAG_REVERSE;
        attrN++;
      }

      if (hime_display_on_the_spot_key() && i==ggg.gbuf_cursor)
        strN += get_DispInArea_str(str+strN);

      memcpy(str+strN, s, len);
      strN+=len;
    }
  }


  if (hime_display_on_the_spot_key() && i==ggg.gbuf_cursor)
    strN += get_DispInArea_str(str+strN);

  str[strN]=0;

  attr[0].ofs1 = ch_N;
  return attrN;
}

extern GtkWidget *gwin_gtab;
void gtab_reset()
{
#if UNIX
  if (!gwin_gtab)
    return;
#endif
  clear_gtab_buf_all();
  clear_gbuf_sel();
  ClrIn();
  return;
}

int ch_to_gtab_keys(INMD *tinmd, char *ch, u_int64_t keys[]);

void save_gtab_buf_phrase_idx(int idx0, int len)
{
  WSP_S wsp[MAX_PHRASE_LEN];

  bzero(wsp, sizeof(wsp));
  int i;
  for(i=0; i < len; i++) {
    u8cpy(wsp[i].ch, gbuf[idx0 + i].ch);
    u_int64_t key = gbuf[idx0 + i].keys[0];

    if (!key) {
      u_int64_t keys[64];
      int keysN = ch_to_gtab_keys(cur_inmd, wsp[i].ch, keys);
      if (keysN)
        key = keys[0];
    }

    wsp[i].key = key;
  }

  create_win_save_phrase(wsp, len);
}

void save_gtab_buf_phrase(KeySym key)
{
  int len = key - '0';
  int idx0 = ggg.gbuf_cursor - len;
  int idx1 = ggg.gbuf_cursor - 1;

  if (idx0 < 0 || idx0 > idx1)
    return;

  save_gtab_buf_phrase_idx(idx0, len);
}

gboolean save_gtab_buf_shift_enter()
{
	int N = ggg.gbufN - ggg.gbuf_cursor;
	if (!N)
		return 0;

	save_gtab_buf_phrase_idx(ggg.gbuf_cursor, N);
	gbuf_cursor_end();
	return 1;
}


void load_tsin_db0(char *infname, gboolean is_gtab_i);
gboolean init_tsin_table_fname(INMD *p, char *fname);

void init_tsin_table()
{
  char fname[256];
  if (!current_CS)
    return;

  init_tsin_table_fname(&inmd[current_CS->in_method], fname);
  load_tsin_db0(fname, TRUE);
}

extern u_char scanphr_e(int chpho_idx, int plen, gboolean pho_incr, int *rselN);
void init_pre_sel();
void clear_sele();
void set_sele_text(int tN, int i, char *text, int len);
void get_win_gtab_geom();
void disp_selections(int x, int y);

gboolean use_tsin_sel_win();
void init_tsin_selection_win();

static int gtab_pre_select_phrase_len;

void disp_gtab_pre_sel(char *s);
extern GtkWidget *gwin1;

void gtab_scan_pre_select(gboolean b_incr)
{
  if (!gtab_phrase_pre_select)
    return;
//  dbg("gtab_scan_pre_select\n");

  tss.pre_selN = 0;

  hide_gtab_pre_sel();

  if (!gtab_cursor_end() || !ggg.gbufN)
    return;

  init_tsin_table();
  init_pre_sel();

  int Maxlen = ggg.gbufN;
  if (Maxlen > MAX_PHRASE_LEN)
    Maxlen = MAX_PHRASE_LEN;

  int len, selN, max_len=-1, max_selN;
  for(len=1; len <= Maxlen; len++) {
    int idx = ggg.gbufN - len;
    if (gbuf[idx].flag & FLAG_CHPHO_PHRASE_TAIL)
      break;
    int mlen = scanphr_e(ggg.gbufN - len, len, b_incr, &selN);
    if (mlen) {
      max_len = len;
      max_selN = selN;
    }
  }

//  dbg("max_len:%d  max_selN:%d\n", max_len, max_selN);

  if (max_len < 0 || max_selN >= strlen(cur_inmd->selkey) * 2) {
    tss.pre_selN = 0;
    return;
  }

  gtab_pre_select_phrase_len = max_len;

  scanphr_e(ggg.gbufN - max_len, max_len, b_incr, &selN);

//  dbg("selN:%d %d\n", selN, tss.pre_selN);

  if (selN==1 && tss.pre_sel[0].len==max_len) {
    char out[MAX_PHRASE_LEN * CH_SZ + 1];
    extract_gbuf_str(ggg.gbufN - max_len, max_len, out);
    if (!strcmp(out, tss.pre_sel[0].str))
      return;
  }

//  dbg("selN %d %d\n",selN, tss.pre_selN);

  if (use_tsin_sel_win()) {
	if (gwin1)
      clear_sele();
	else
      init_tsin_selection_win();

    int i;
    for(i=0;i<tss.pre_selN; i++)
       set_sele_text(tss.pre_selN,i,tss.pre_sel[i].str, -1);
    get_win_gtab_geom();
    disp_selections(-1, -1);
    return;
  }

  char tt[4096];
  tt[0]=0;
  int i;

  for(i=0;i<tss.pre_selN; i++) {
    char ts[(MAX_PHRASE_LEN+3) * CH_SZ + 1];
    char *br= (i < tss.pre_selN-1 && gtab_vertical_select)?"\n":"";

    sprintf(ts, "<span foreground=\"%s\">%c</span>%s%s", hime_sel_key_color,
      cur_inmd->selkey[i], tss.pre_sel[i].str, br);
    strcat(tt, ts);
    if (!gtab_vertical_select && i < tss.pre_selN-1)
      strcat(tt, " ");
  }

//  dbg("tt %s\n", tt);
  disp_gtab_pre_sel(tt);
}


int shift_key_idx(char *s, KeySym xkey);

gboolean gtab_pre_select_idx(int c)
{
  if (c < 0)
    return FALSE;
  if (c >= tss.pre_selN)
    return TRUE;

#if 0
  dbg("c %d %s  ggg.gbuf_cursor:%d,%d\n", c, tss.pre_sel[c].str,
    ggg.gbuf_cursor, ggg.gbufN);
#endif

  gtab_buf_backspaceN(gtab_pre_select_phrase_len);
  int len = tss.pre_sel[c].len;
  insert_gbuf_cursor_phrase(tss.pre_sel[c].str, tss.pre_sel[c].phkey, len);
  gbuf[ggg.gbufN-1].flag |= FLAG_CHPHO_PHRASE_TAIL;

  hide_gtab_pre_sel();
  if (hime_edit_display_ap_only())
    hide_win_gtab();

  return TRUE;
}

gboolean gtab_pre_select_shift(KeySym key, int kbstate)
{
//  dbg("gtab_pre_select_shift %c\n", key);
  if (!gtab_phrase_pre_select || !tss.pre_selN)
    return FALSE;

  int c = shift_key_idx(cur_inmd->selkey, key);
  return gtab_pre_select_idx(c);
}

void tsin_toggle_eng_ch();

int feedkey_gtab_release(KeySym xkey, int kbstate)
{
  gint64 kpt;

  switch (xkey) {
     case XK_Control_L:
     case XK_Control_R:
       kpt = key_press_time_ctrl;
       key_press_time_ctrl = 0;
        if (current_time() - kpt < 300000 && tss.pre_selN) {
          if (!test_mode) {
            tss.ctrl_pre_sel = TRUE;
          }
          return 1;
        } else
          return 0;
#if 1
     case XK_Shift_L:
     case XK_Shift_R:
       kpt = key_press_time;
       key_press_time = 0;

// dbg("release xkey %x\n", xkey);
        if (
(  (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Shift) ||
   (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftL
     && xkey == XK_Shift_L) ||
   (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftR
     && xkey == XK_Shift_R))
          &&  current_time() - kpt < 300000) {
          if (!test_mode) {
            tsin_toggle_eng_ch();
          }
          return 1;
        } else
          return 0;
#endif
     default:
        return 0;
  }
}

#include "win1.h"

void gtab_set_win1_cb()
{
  set_win1_cb((cb_selec_by_idx_t)gtab_pre_select_idx, NULL, NULL);
}
