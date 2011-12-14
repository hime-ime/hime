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
#include "pho.h"
#include "config.h"
#if HIME_i18n_message
#include <libintl.h>
#endif
#include "gst.h"
#include "tsin.h"

static gboolean b_pinyin;
#if !HIME_SVR
PIN_JUYIN *pin_juyin;
int pin_juyinN;
PHOKBM phkbm;
PHO_ST poo;
TSIN_ST tss;
int text_pho_N;

void key_typ_pho(phokey_t phokey, u_char rtyp_pho[])
{
}
#endif

char *phokey2pinyin(phokey_t k)
{
  static char tt[32];
  phokey_t tonemask = 7;
  phokey_t notone = k & ~tonemask;


  int i;
  for(i=0; i < pin_juyinN; i++) {
    if (notone == pin_juyin[i].key)
      break;
  }

  if (notone && i==pin_juyinN) {
//    prph(k);
    strcpy(tt, "??");
  } else {
static char tone[2];
    tone[0] = (k & tonemask) + '0';
    strcpy(tt, pin_juyin[i].pinyin);

    if (tone[0]=='1')
      tone[0]='5';

    if (tone[0]!='0')
      strcat(tt, tone);
  }

  return tt;
}

void load_pin_juyin();

gboolean is_pinyin_kbm()
{
  char kbm_str[32];

  get_hime_conf_fstr(PHONETIC_KEYBOARD, kbm_str, "zo-asdf");
#if 1
  b_pinyin = strstr(kbm_str, "pinyin") != NULL;
#else
  b_pinyin = 1;
#endif

  if (b_pinyin)
    load_pin_juyin();
  return b_pinyin;
}


phokey_t pinyin2phokey(char *s)
{
  char *p = s;
  while (*p && *p!=' ')
    p++;
  int len = p - s;
  char tone = s[len-1];

  if (tone<'1' || tone > '5')
    tone = 0;
  else {
    tone -= '0';
    if (tone==5)
      tone = 1;
  }

  if (len==1 && tone)
    return tone;

//  dbg("'%s' '%d'\n", s, tone);

  int mlen = tone ? len-1:len;

  char t[16];

  memcpy(t, s, mlen);
  t[mlen]=0;

  int i;
  for(i=0; i < pin_juyinN; i++) {
    if (!strcmp(pin_juyin[i].pinyin, t)) {
      return (pin_juyin[i].key | tone);
    }
  }

  return 0;
}
