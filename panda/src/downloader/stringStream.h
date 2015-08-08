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
#include "extension.h"

////////////////////////////////////////////////////////////////////
//       Class : StringStream
// Description : A bi-directional stream object that reads and writes
//               data to an internal buffer, which can be retrieved
//               and/or set as a string in Python 2 or a bytes object
//               in Python 3.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS StringStream : public iostream {
public:
  INLINE StringStream(const string &source);

PUBLISHED:
  EXTENSION(StringStream(PyObject *source));
  INLINE StringStream();

  INLINE void clear_data();
  INLINE size_t get_data_size();

  EXTENSION(PyObject *get_data());
  EXTENSION(void set_data(PyObject *data));

  MAKE_PROPERTY(data, get_data, set_data);

public:
#ifndef CPPPARSER
  INLINE string get_data();
  INLINE void set_data(const string &data);
#endif

  INLINE void swap_data(pvector<unsigned char> &data);

private:
  StringStreamBuf _buf;

  friend class Extension<StringStream>;
};

#include "stringStream.I"

#endif

