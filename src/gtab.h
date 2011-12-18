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

typedef enum {
  GTAB_space_auto_first_none=0,   // use the value set by .cin
  GTAB_space_auto_first_any=1,    // boshiamy, dayi
  GTAB_space_auto_first_full=2,   // simplex
  GTAB_space_auto_first_nofull=4,  // windows ar30 cj
  GTAB_space_auto_first_dayi=8    // dayi: input:2   select:1
} GTAB_space_pressed_E;

typedef struct {
  u_char key[4];   /* If I use u_long key, the struc size will be 8 */
  u_char ch[CH_SZ];
} ITEM;

typedef struct {
  u_char key[8];   /* If I use u_long key, the struc size will be 8 */
  u_char ch[CH_SZ];
} ITEM64;

typedef struct {
  char quick1[46][10][CH_SZ];
  char quick2[46][46][10][CH_SZ];
} QUICK_KEYS;


enum {
  FLAG_KEEP_KEY_CASE=1,
  FLAG_GTAB_SYM_KBM=2, // auto close, auto switch to default input method
  FLAG_PHRASE_AUTO_SKIP_ENDKEY=4,
  FLAG_AUTO_SELECT_BY_PHRASE=8,
  FLAG_GTAB_DISP_PARTIAL_MATCH=0x10,
};

enum {
  GTAB_AUTO_SELECT_BY_PHRASE_AUTO=0,
  GTAB_AUTO_SELECT_BY_PHRASE_YES=1,
  GTAB_AUTO_SELECT_BY_PHRASE_NO=2,
};


#define MAX_SELKEY 16

struct TableHead {
  int version;
  u_int flag;
  char cname[32];         /* prompt */
  char selkey[12];        /* select keys */
  GTAB_space_pressed_E space_style;
  int KeyS;               /* number of keys needed */
  int MaxPress;           /* Max len of keystroke  ar30:4  changjei:5 */
  int M_DUP_SEL;          /* how many keys used to select */
  int DefC;               /* Defined characters */
  QUICK_KEYS qkeys;

  union {
    struct {
      char endkey[99];
      char keybits;
      char selkey2[10];
    };

    char dummy[128];  // for future use
  };
};


#define KeyBits1(inm) (inm->keybits)
#define KeyBits (cur_inmd->keybits)
#define MAX_GTAB_KEYS (1<<KeyBits)

#define MAX_TAB_KEY_NUM (32/KeyBits)
#define MAX_TAB_KEY_NUM1(inm) (32/KeyBits1(inm))
#define MAX_TAB_KEY_NUM64 (64/KeyBits)
#define MAX_TAB_KEY_NUM641(inm) (64/KeyBits1(inm))

struct _HIME_module_callback_functions;
typedef u_int gtab_idx1_t;

typedef struct {
  ITEM *tbl;
  ITEM64 *tbl64;
  QUICK_KEYS *qkeys;
  int use_quick;
  u_int flag;
#define MAX_CNAME (4*CH_SZ+1)
  char *cname;
  char *keycol;
  int KeyS;               /* number of keys needed */
  int MaxPress;           /* Max len of keystrike  ar30:5  changjei:5 */
  int DefChars;           /* defined chars */
  char *keyname; // including ?*
  char *keyname_lookup; // used by boshiamy only
  gtab_idx1_t *idx1;
  char *keymap;
  char *selkey;
  u_char *sel1st;
  int M_DUP_SEL;
  int phrnum;
  int *phridx;
  char *phrbuf;
  char *filename, *filename_append;
  time_t file_modify_time;
  gboolean key64; // db is 64 bit-long key
  gboolean disabled; // will not be display in the selection menu
  int max_keyN;
  char *endkey;       // only pinin/ar30 use it
  GTAB_space_pressed_E space_style;
  char *icon;
  u_char kmask, keybits, last_k_bitn, method_type;
  char WILD_QUES, WILD_STAR;
  struct _HIME_module_callback_functions *mod_cb_funcs;
  char key_ch, in_cycle;
} INMD;

enum {
  method_type_GTAB=1,
  method_type_PHO=3,
  method_type_TSIN=6,
  method_type_MODULE=12,
  method_type_SYMBOL_TABLE=13,
};

extern INMD *inmd;
extern int inmdN;

u_int64_t CONVT2(INMD *inmd, int i);
extern INMD *cur_inmd;
void load_gtab_list(gboolean);
char current_method_type();

#define LAST_K_bitN (cur_inmd->last_k_bitn)

#define KEY_MASK ((1<<cur_inmd->keybits)-1);


#define GTAB_LIST "gtab.list"

#if 1
#define NEED_SWAP (__BYTE_ORDER == __BIG_ENDIAN && 0)
#else
#define NEED_SWAP (1)
#endif

#define tblch2(inm, i) (inm->key64 ? inm->tbl64[i].ch:inm->tbl[i].ch)
#define Max_tab_key_num1(inm) (inm->key64 ? MAX_TAB_KEY_NUM641(inm) : MAX_TAB_KEY_NUM1(inm))
#define Max_tab_key_num Max_tab_key_num1(cur_inmd)
