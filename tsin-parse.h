typedef struct {
  char len, flag;
  u_char start;
  unsigned char str[MAX_PHRASE_LEN*CH_SZ+1];  // use malloc
} TSIN_PARSE;

enum {
  FLAG_TSIN_PARSE_PHRASE = 1,
  FLAG_TSIN_PARSE_PARTIAL = 2, //partial phrase
};

typedef struct {
  int start;
  int usecount;
  short match_phr_N, no_match_ch_N;
  TSIN_PARSE best[MAX_PH_BF_EXT+1];
} CACHE;

void tsin_parse();
void init_cache(int tc_len);
CACHE *cache_lookup(int start);
int tsin_parse_recur(int start, TSIN_PARSE *out,
                     short *r_match_phr_N, short *r_no_match_ch_N);
