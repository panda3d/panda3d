// Filename: stringStreamBuf.h
// Created by:  drose (02Jul07)
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

#ifndef STRINGSTREAMBUF_H
#define STRINGSTREAMBUF_H

#include "pandabase.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : StringStreamBuf
// Description : Used by StringStream to implement an stream that
//               reads from and/or writes to a memory buffer, whose
//               contents can be appended to or extracted at any time
//               by application code.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS StringStreamBuf : public streambuf {
public:
  StringStreamBuf();
  virtual ~StringStreamBuf();

  void clear();

  INLINE void swap_data(pvector<unsigned char> &data);
  INLINE const pvector<unsigned char> &get_data() const;

  size_t read_chars(char *start, size_t length);
  void write_chars(const char *start, size_t length);

protected:
  virtual streampos seekoff(streamoff off, ios_seekdir dir, ios_openmode which);
  virtual streampos seekpos(streampos pos, ios_openmode which);

  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();

private:
  pvector<unsigned char> _data;
  char *_buffer;
  size_t _ppos;
  size_t _gpos;
};

#include "stringStreamBuf.I"

#endif
