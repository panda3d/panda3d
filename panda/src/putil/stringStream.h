// Filename: stringStream.h
// Created by:  drose (03Jul07)
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

#ifndef STRINGSTREAM_H
#define STRINGSTREAM_H

#include "pandabase.h"
#include "stringStreamBuf.h"

////////////////////////////////////////////////////////////////////
//       Class : StringStream
// Description : A bi-directional stream object that reads and writes
//               data to an internal buffer, which can be appended to
//               or read from as a string.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA StringStream : public iostream {
PUBLISHED:
  INLINE StringStream();
  INLINE StringStream(const string &source);

  INLINE void clear_data();
  INLINE size_t get_data_size();

  INLINE string get_data();
  INLINE void set_data(const string &data);
  INLINE void swap_data(pvector<unsigned char> &data);

private:
  StringStreamBuf _buf;
};

#include "stringStream.I"

#endif

