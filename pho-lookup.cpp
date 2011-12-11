/* Copyright (C) 2009 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

static int shiftb[]={9,7,3,0};

int lookup(u_char *s)
{
  int i;
  char tt[CH_SZ+1], *pp;

  if (*s < 128)
    return *s-'0';

  bchcpy(tt, s);
  tt[PHO_CHAR_LEN]=0;

  for(i=0;i<3;i++)
    if ((pp=strstr(pho_chars[i],tt)))
      break;

  if (i==3)
    return 0;

  return (((pp-pho_chars[i])/3) << shiftb[i]);
}
