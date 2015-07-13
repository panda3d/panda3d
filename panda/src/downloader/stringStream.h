// Filename: stringStream.h
// Created by:  drose (03Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef STRINGSTREAM_H
#define STRINGSTREAM_H

#include "pandabase.h"
#include "stringStreamBuf.h"

////////////////////////////////////////////////////////////////////
//       Class : StringStream
// Description : A bi-directional stream object that reads and writes
//               data to an internal buffer, which can be retrieved
//               and/or set as a string.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS StringStream : public iostream {
PUBLISHED:
  INLINE StringStream();
  INLINE StringStream(const string &source);

  INLINE void clear_data();
  INLINE size_t get_data_size();

  INLINE string get_data();
  INLINE void set_data(const string &data);

public:
  INLINE void swap_data(pvector<unsigned char> &data);

private:
  StringStreamBuf _buf;
};

#include "stringStream.I"

#endif

