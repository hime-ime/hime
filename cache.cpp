#include <string.h>
#include "hime.h"
#include "pho.h"
#include "tsin.h"
#include "hime-conf.h"
#include "tsin-parse.h"

static CACHE *cache;
static int cacheN;

CACHE *cache_lookup(int start)
{
  int i;

  for(i=0; i < cacheN; i++)
    if (cache[i].start == start)
      return &cache[i];
  return NULL;
}

void add_cache(int start, int usecount, TSIN_PARSE *out,
                      short match_phr_N, short no_match_ch_N, int tc_len)
{
  cache[cacheN].start = start;
  cache[cacheN].usecount = usecount;
  cache[cacheN].match_phr_N = match_phr_N;
  cache[cacheN].no_match_ch_N = no_match_ch_N;
  memcpy(cache[cacheN].best, out, sizeof(TSIN_PARSE) * (tc_len - start));
  cacheN++;
}


void init_cache(int tc_len)
{
  cache = tmalloc(CACHE, tc_len);
  cacheN = 0;
}

void free_cache()
{
  free(cache); cache = NULL;
}
