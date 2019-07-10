/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stringStream.h
 * @author drose
 * @date 2007-07-03
 */

#ifndef STRINGSTREAM_H
#define STRINGSTREAM_H

#include "pandabase.h"
#include "stringStreamBuf.h"
#include "vector_uchar.h"
#include "extension.h"

/**
 * A bi-directional stream object that reads and writes data to an internal
 * buffer, which can be retrieved and/or set as a string in Python 2 or a
 * bytes object in Python 3.
 */
class EXPCL_PANDA_EXPRESS StringStream : public std::iostream {
public:
  INLINE StringStream(const std::string &source);
  INLINE StringStream(vector_uchar source);

PUBLISHED:
  EXTENSION(StringStream(PyObject *source));
  INLINE StringStream();

#if _MSC_VER >= 1800
  INLINE StringStream(const StringStream &copy) = delete;
#endif

  INLINE void clear_data();
  INLINE size_t get_data_size();

  EXTENSION(PyObject *get_data());
  EXTENSION(void set_data(PyObject *data));

  MAKE_PROPERTY(data, get_data, set_data);

public:
#ifndef CPPPARSER
  INLINE std::string get_data();
  INLINE void set_data(const std::string &data);
  void set_data(const unsigned char *data, size_t size);
#endif

  INLINE void swap_data(vector_uchar &data);

private:
  StringStreamBuf _buf;

  friend class Extension<StringStream>;
};

#include "stringStream.I"

#endif
