// Filename: pnmimage_base.cxx
// Created by:  drose (04Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pnmimage_base.h"
#include "streamReader.h"
#include "streamWriter.h"

#include <stdarg.h>
#include <stdio.h>   // for sprintf()


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
#ifdef WIN32_VC
  // Windows doesn't define vsnprintf().  Hope we don't overflow.
  vsprintf(buffer, format, ap);
#else
  vsnprintf(buffer, buffer_size, format, ap);
#endif
  nassertv(strlen(buffer) < buffer_size);

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
#ifdef WIN32_VC
  // Windows doesn't define vsnprintf().  Hope we don't overflow.
  vsprintf(buffer, format, ap);
#else
  vsnprintf(buffer, buffer_size, format, ap);
#endif
  nassertv(strlen(buffer) < buffer_size);

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
pm_readbigshort(istream *in, short *sP) {
  StreamReader reader(in, false);
  *sP = reader.get_be_int16();
  return (!in->eof() && !in->fail()) ? 0 : -1;
}

int
pm_writebigshort(ostream *out, short s) {
  StreamWriter writer(out);
  writer.add_be_int16(s);
  return (!out->fail()) ? 0 : -1;
}

int
pm_readbiglong(istream *in, long *lP) {
  StreamReader reader(in, false);
  *lP = reader.get_be_int32();
  return (!in->eof() && !in->fail()) ? 0 : -1;
}

int
pm_writebiglong(ostream *out, long l) {
  StreamWriter writer(out);
  writer.add_be_int32(l);
  return (!out->fail()) ? 0 : -1;
}

int
pm_readlittleshort(istream *in, short *sP) {
  StreamReader reader(in, false);
  *sP = reader.get_int16();
  return (!in->eof() && !in->fail()) ? 0 : -1;
}

int
pm_writelittleshort(ostream *out, short s) {
  StreamWriter writer(out);
  writer.add_int16(s);
  return (!out->fail()) ? 0 : -1;
}

int
pm_readlittlelong(istream *in, long *lP) {
  StreamReader reader(in, false);
  *lP = reader.get_int32();
  return (!in->eof() && !in->fail()) ? 0 : -1;
}

int
pm_writelittlelong(ostream *out, long l) {
  StreamWriter writer(out);
  writer.add_int32(l);
  return (!out->fail()) ? 0 : -1;
}
