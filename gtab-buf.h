typedef struct {
  char *ch;
  char **sel;
  int selN;
  u_short flag;
  u_char c_sel;
  char plen, keysN;
  u_int64_t keys[8];
} GEDIT;

extern GEDIT *gbuf;
extern short gbufN;

void insert_gbuf_nokey(char *s);
void insert_gbuf_cursor1_cond(char *s, u_int64_t key, gboolean valid_key);
GEDIT *insert_gbuf_cursor(char **sel, int selN, u_int64_t key, gboolean b_gtab_en_no_spc);
