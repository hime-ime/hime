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

#if !HIME_IME && !TSF
void utf8_big5_n(char *s, int len, char out[])
{
  out[0]=0;

  GError *err = NULL;
  gsize rn, wn;
  char *big5 = g_locale_from_utf8 (s, len, &rn, &wn, &err);

  if (err || !big5) {
    dbg("utf8_big5 convert error\n");
//    abort();
    return;
  }

  strcpy(out, big5);
  g_free(big5);
}


void utf8_big5(char *s, char out[])
{
  utf8_big5_n(s, strlen(s), out);
}
#endif


int utf8_sz(char *s)
{
  if (!(*s & 0x80))
    return 1;

  if ((*s & 0xe0) == 0xc0)
    return 2;

  if ((*s & 0xf0) == 0xe0)
    return 3;

  if ((*s & 0xf8) == 0xf0)
    return 4;

  p_err("bad utf8 char %x %c%c%c", *s, *s, *(s+1), *(s+2));
  return -1;
}


int utf8cpy(char *t, char *s)
{
  int utf8sz = utf8_sz(s);

  memcpy(t, s, utf8sz);
  t[utf8sz] = 0;
  return utf8sz;
}

// copy N utf-8 chars
void utf8cpyN(char *t, char *s, int N)
{
  int len = utf8_tlen(s, N);

  memcpy(t, s, len);

  t[len] = 0;
}


int u8cpy(char *t, char *s)
{
  int utf8sz = utf8_sz(s);

  memcpy(t, s, utf8sz);
  return utf8sz;
}


int utf8_tlen(char *s, int N)
{
  int i;
  char *p = s;

  for(i=0; i < N; i++) {
    int len = utf8_sz(p);
    p+=len;
  }

  return p - s;
}

int utf8_to_big5(char *in, char *out, int outN);
void utf8_putchar_fp(FILE *fp, char *s)
{
  int i;
  int len = utf8_sz(s);
  for(i=0;i<len;i++)
    fputc(s[i], fp);
}


void utf8_putchar(char *s)
{
#if WIN32
  char tt[CH_SZ+1], vv[CH_SZ+1];
  utf8cpy(tt, s);
  int len = utf8_to_big5(tt, vv, sizeof(vv));
  for(int i=0;i<len;i++)
    fputc(vv[i], stdout);
#else
	utf8_putchar_fp(stdout, s);
#endif
}

void utf8_putcharn(char *s, int n)
{
  int i, ofs;

  for(ofs=i=0; i < n; i++) {
    utf8_putchar(&s[ofs]);
    ofs+= utf8_sz(&s[ofs]);
  }
}

gboolean utf8_eq(char *a, char *b)
{
  int ta = utf8_sz(a);
  int tb = utf8_sz(b);

  if (ta != tb)
    return FALSE;

  return !memcmp(a,b, ta);
}

gboolean utf8_str_eq(char *a, char *b, int len)
{
  int ta = utf8_tlen(a, len);
  int tb = utf8_tlen(b, len);

  if (ta != tb)
    return FALSE;

  return !memcmp(a, b, ta);
}

int utf8_str_N(char *str)
{
  int N=0;

  while (*str) {
    str+= utf8_sz(str);
    N++;
  }

  return N;
}

// copy at most n utf-8 chars
void utf8cpyn(char *t, char *s, int n)
{
  int tn=0;
  int i;

  for (i=0; i < n && *s; i++) {
    int sz = utf8_sz(s);

    memcpy(t+tn, s, sz);
    tn+=sz;
    s+=sz;
  }

  t[tn]=0;
}


// copy at most utf-8 bytes
void utf8cpy_bytes(char *t, char *s, int n)
{
  int tn=0;
  int i;

  for (i=0; tn < n && *s; i++) {
    int sz = utf8_sz(s);

    memcpy(t+tn, s, sz);
    tn+=sz;
    s+=sz;
  }

  t[tn]=0;
}

#if WIN32
int utf8_to_16(char *text, wchar_t *wtext, int wlen)
{
  return MultiByteToWideChar( CP_UTF8, 0, text, -1, wtext, wlen/sizeof(wchar_t)) -1;
}

int utf16_to_8(wchar_t *in, char *out, int outN)
{
  return WideCharToMultiByte( CP_UTF8, 0, in, -1, out, outN, NULL, NULL) - 1;
}

int utf8_to_big5(char *in, char *out, int outN)
{
	wchar_t tt[512];
	utf8_to_16(in, tt, sizeof(tt));
	return WideCharToMultiByte( 950, 0, tt, -1, out, outN, NULL, NULL) - 1;
}


char *__utf16_8(wchar_t *s)
{
  static char tt[512];
  utf16_to_8(s, tt, sizeof(tt));
  return tt;
}

#endif


char utf8_sigature[]="\xef\xbb\xbf";

void skip_utf8_sigature(FILE *fp)
{
	char tt[3];

	tt[0]=0;
	fread(tt, 1, 3, fp);
	if (memcmp(tt, utf8_sigature, 3)) {
//		fseek(fp, 0, SEEK_SET);
		rewind(fp);
	}
}
