// Filename: pnmimage_base.cxx
// Created by:  drose (04Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pnmimage_base.h"

#include <stdarg.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////
//     Function: pm_message
//  Description: Outputs the given printf-style message to the user
//               and returns.
////////////////////////////////////////////////////////////////////
void
pm_message(const char *format, ...) {
  va_list ap;
  va_start(ap, format);

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];
  vsnprintf(buffer, buffer_size, format, ap);

  pnmimage_cat.info() << buffer << "\n";

  va_end(ap);
}

////////////////////////////////////////////////////////////////////
//     Function: pm_error
//  Description: Outputs the given printf-style message to the user
//               and terminates messily.  Minimize use of this
//               function.
////////////////////////////////////////////////////////////////////
void
pm_error(const char *format, ...) {
  va_list ap;
  va_start(ap, format);

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];
  vsnprintf(buffer, buffer_size, format, ap);

  pnmimage_cat.error() << buffer << "\n";

  va_end(ap);

  // Now we're supposed to exit.  Inconvenient if we were running
  // Panda interactively, but that's the way it is.
  exit(1);
}

////////////////////////////////////////////////////////////////////
//     Function: pm_maxvaltobits
//  Description: Returns the number of bits sufficient to hold the
//               indicated maxval value.
////////////////////////////////////////////////////////////////////
int
pm_maxvaltobits(int maxval) {
  int bits = 1;
  while (maxval > pm_bitstomaxval(bits)) {
    bits++;
    nassertr(bits != 0, 16);
  }
  return bits;
}

////////////////////////////////////////////////////////////////////
//     Function: pm_bitstomaxval
//  Description: Returns the highest maxval that can be represented in
//               the indicated number of bits.
////////////////////////////////////////////////////////////////////
int
pm_bitstomaxval(int bits) {
  return ( 1 << bits ) - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: pm_allocrow
//  Description: Allocates a row of cols * size bytes.
////////////////////////////////////////////////////////////////////
char *
pm_allocrow(int cols, int size) {
  return new char[cols * size];
}

////////////////////////////////////////////////////////////////////
//     Function: pm_freerow
//  Description: Frees the row previously allocated withm pm_allocrow().
////////////////////////////////////////////////////////////////////
void
pm_freerow(char *itrow) {
  delete[] itrow;
}


/* Endian I/O.
*/

int
pm_readbigshort(FILE* in, short* sP)
    {
    int c;

    if ( (c = getc( in )) == EOF )
        return -1;
    *sP = ( c & 0xff ) << 8;
    if ( (c = getc( in )) == EOF )
        return -1;
    *sP |= c & 0xff;
    return 0;
    }

int
pm_writebigshort( FILE* out, short s )
    {
    (void) putc( ( s >> 8 ) & 0xff, out );
    (void) putc( s & 0xff, out );
    return 0;
    }

int
pm_readbiglong(FILE* in, long* lP)
    {
    int c;

    if ( (c = getc( in )) == EOF )
        return -1;
    *lP = ( c & 0xff ) << 24;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= ( c & 0xff ) << 16;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= ( c & 0xff ) << 8;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= c & 0xff;
    return 0;
    }

int
pm_writebiglong(FILE* out, long l)
    {
    (void) putc( ( l >> 24 ) & 0xff, out );
    (void) putc( ( l >> 16 ) & 0xff, out );
    (void) putc( ( l >> 8 ) & 0xff, out );
    (void) putc( l & 0xff, out );
    return 0;
    }

int
pm_readlittleshort(FILE* in, short* sP)
    {
    int c;

    if ( (c = getc( in )) == EOF )
        return -1;
    *sP = c & 0xff;
    if ( (c = getc( in )) == EOF )
        return -1;
    *sP |= ( c & 0xff ) << 8;
    return 0;
    }

int
pm_writelittleshort( FILE* out, short s )
    {
    (void) putc( s & 0xff, out );
    (void) putc( ( s >> 8 ) & 0xff, out );
    return 0;
    }

int
pm_readlittlelong(FILE* in, long* lP)
    {
    int c;

    if ( (c = getc( in )) == EOF )
        return -1;
    *lP = c & 0xff;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= ( c & 0xff ) << 8;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= ( c & 0xff ) << 16;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= ( c & 0xff ) << 24;
    return 0;
    }

int
pm_writelittlelong(FILE* out, long l)
    {
    (void) putc( l & 0xff, out );
    (void) putc( ( l >> 8 ) & 0xff, out );
    (void) putc( ( l >> 16 ) & 0xff, out );
    (void) putc( ( l >> 24 ) & 0xff, out );
    return 0;
    }
