#include "hime.h"
#include "pho-kbm-name.h"

struct PHO_KBM_NAME kbm_sel[]= {
// {N_(_L("標準 standard")), "zo"},
 {N_(_L("標準")), "zo"},
 {N_(_L("倚天")), "et"},
 {N_(_L("倚天 26 鍵")), "et26"},
 {N_(_L("許氏(國音,自然)")), "hsu"},
 {N_(_L("拼音")), "pinyin"},
 {N_(_L("拼音無聲調")), "pinyin-no-tone"},
 {N_(_L("Dvorak")), "dvorak"},
 {N_(_L("IBM")), "ibm"},
 {N_(_L("神通")), "mitac"},
 {N_(_L("Colemak")), "colemak"},
 {NULL, NULL}
};
