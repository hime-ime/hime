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

#define MAX_CHS (100000)

typedef struct {
  u_short key;
  u_char ch[CH_SZ];
  short count;
  int oseq;
} PHITEM;

PHITEM items[MAX_CHS];
int itemsN;

PHO_ITEM pho_items[MAX_CHS];
int pho_itemsN=0;

int qcmp_key(const void *aa, const void *bb)
{
  PHITEM *a=(PHITEM *)aa;
  PHITEM *b=(PHITEM *)bb;

  int d;
  if ((d=a->key - b->key))
    return a->key - b->key;

  if ((d = b->count - a->count))
    return d;

  return a->oseq - b->oseq;
}


int qcmp_key_del(const void *aa, const void *bb)
{
  PHITEM *a=(PHITEM *)aa;
  PHITEM *b=(PHITEM *)bb;

  int d;
  if ((d=a->key - b->key))
    return a->key - b->key;

  return memcmp(a->ch, b->ch, CH_SZ);
}

void send_hime_message(Display *dpy, char *s);

int main(int argc, char **argv)
{
  char *fname = "pho.tab2.src";
  FILE *fp;
  char s[64];
  int phrase_area_N=0;
  char *phrase_area = NULL;

  if (!getenv("NO_GTK_INIT"))
    gtk_init(&argc, &argv);

  if (argc > 1)
    fname = argv[1];

  if ((fp=fopen(fname,"rb"))==NULL)
    p_err("cannot open %s\n", fname);


  while (!feof(fp)) {
    s[0]=0;
    myfgets(s,sizeof(s),fp);
    int len=strlen(s);

    if (s[len-1]=='\n')
      s[--len]=0;

    if (len==0)
      continue;

    phokey_t kk=0;
    char *p = s;

    while (*p && *p!=' ' && *p!=9) {
      if (kk==(BACK_QUOTE_NO << 9))
        kk|=*p;
      else
        kk |= lookup((u_char *)p);

      p += utf8_sz(p);
    }

    items[itemsN].key = kk;

    p++;

    char *str = p;
    while (*p && *p != ' ' && *p!=9)
      p++;

    *p = 0;
    p++;

    int slen = strlen(str);
    if (slen==utf8_sz(str)) {
      u8cpy((char *)items[itemsN].ch, str);
    } else {
      dbg("str %s\n", str);
      int newN = phrase_area_N + slen + 1;
      phrase_area = trealloc(phrase_area, char, newN);
      strcpy(phrase_area + phrase_area_N, str);
      items[itemsN].ch[0] = PHO_PHRASE_ESCAPE;
      items[itemsN].ch[1] = phrase_area_N & 0xff;
      items[itemsN].ch[2] = (phrase_area_N>>8) & 0xff;
      items[itemsN].ch[3] = (phrase_area_N>>16) & 0xff;
      phrase_area_N = newN;
    }

    items[itemsN].count = atoi(p);
    items[itemsN].oseq = itemsN;

    itemsN++;
  }

  fclose(fp);


  qsort(items, itemsN, sizeof(PHITEM), qcmp_key_del);
  int i;

#if 1
  int newN = 1;
  for(i=1;i<itemsN;i++)
    if (qcmp_key_del(&items[i-1], &items[i]))
      items[newN++] = items[i];
    else {
#if 0
      prph(items[i].key);
      utf8_putchar((char *)items[i].ch);
      dbg("\n");
#endif
    }

  if (itemsN != newN) {
    dbg("deleted %d %d\n",itemsN, newN);
    itemsN = newN;
  }
#endif

  qsort(items, itemsN, sizeof(PHITEM), qcmp_key);

  PHO_IDX pho_idx[3000];
  u_short pho_idxN=0;

  for(i=0; i < itemsN; ) {
    phokey_t key = items[i].key;
    pho_idx[pho_idxN].key = key;
    pho_idx[pho_idxN].start = i;
    pho_idxN++;

    int j;

    for (j=i+1; j < itemsN && items[j].key == key; j++);

    int l;
    for(l=i; l<j; l++) {
      bchcpy(pho_items[pho_itemsN].ch, items[l].ch);
      pho_items[pho_itemsN].count = items[l].count;
      pho_itemsN++;
    }

    i = j;
  }

  char *tp = strstr(fname, ".tab2.src");
  if (!tp)
    p_err("file name should be *.tab2.src");

  tp = strstr(fname, ".src");
  *tp=0;

  char *fname_out = fname;

  if ((fp=fopen(fname_out,"wb"))==NULL)
    p_err("cannot create %s\n", fname_out);

  fwrite("PH",1,2,fp);
//  dbg("pho_itemsN:%d  pho_idxN:%d\n", pho_itemsN, pho_idxN);
  fwrite(&pho_idxN, sizeof(u_short), 1, fp);
  fwrite(&pho_itemsN, sizeof(pho_itemsN), 1, fp);
  fwrite(&phrase_area_N, sizeof(phrase_area_N), 1, fp);
#if 0
  fclose(fp); exit(0);
#endif
  fwrite(pho_idx, sizeof(PHO_IDX), pho_idxN, fp);
  fwrite(pho_items, sizeof(PHO_ITEM), pho_itemsN, fp);

  fwrite(phrase_area, 1, phrase_area_N, fp);

  fclose(fp);

  if (getenv("HIME_NO_RELOAD")==NULL) {
    send_hime_message(
#if UNIX
	GDK_DISPLAY(),
#endif
	"reload");
  }

  return 0;
}
