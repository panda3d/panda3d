/* Filename: getbits.h
 * Created by:  
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
 *
 * All use of this software is subject to the terms of the Panda 3d
 * Software license.  You should have received a copy of this license
 * along with this source code; you will also find a current copy of
 * the license at http://www.panda3d.org/license.txt .
 *
 * To contact the maintainers of this program write to
 * panda3d@yahoogroups.com .
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static unsigned long rval;
static unsigned char rval_uc;

#define backbits(nob) ((void)( \
  bsi.bitindex    -= (nob), \
  bsi.wordpointer += (bsi.bitindex>>3), \
  bsi.bitindex    &= 0x7 ))

#define getbitoffset() ((-bsi.bitindex)&0x7)
#define getbyte()      (*bsi.wordpointer++)

#define getbits(nob) ( \
  rval = bsi.wordpointer[0], rval <<= 8, rval |= bsi.wordpointer[1], \
  rval <<= 8, rval |= bsi.wordpointer[2], rval <<= bsi.bitindex, \
  rval &= 0xffffff, bsi.bitindex += (nob), \
  rval >>= (24-(nob)), bsi.wordpointer += (bsi.bitindex>>3), \
  bsi.bitindex &= 7,rval)

#define getbits_fast(nob) ( \
  rval = (unsigned char) (bsi.wordpointer[0] << bsi.bitindex), \
  rval |= ((unsigned long) bsi.wordpointer[1]<<bsi.bitindex)>>8, \
  rval <<= (nob), rval >>= 8, \
  bsi.bitindex += (nob), bsi.wordpointer += (bsi.bitindex>>3), \
  bsi.bitindex &= 7, rval )

#define get1bit() ( \
  rval_uc = *bsi.wordpointer << bsi.bitindex, bsi.bitindex++, \
  bsi.wordpointer += (bsi.bitindex>>3), bsi.bitindex &= 7, rval_uc>>7 )

