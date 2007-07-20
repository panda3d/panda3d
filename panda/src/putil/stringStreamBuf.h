// Filename: stringStreamBuf.h
// Created by:  drose (02Jul07)
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
class EXPCL_PANDA_PUTIL StringStreamBuf : public streambuf {
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
