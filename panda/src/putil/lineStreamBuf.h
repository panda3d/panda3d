// Filename: lineStreamBuf.h
// Created by:  drose (26Feb00)
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

#ifndef LINESTREAMBUF_H
#define LINESTREAMBUF_H

#include "pandabase.h"

#include <string>

////////////////////////////////////////////////////////////////////
//       Class : LineStreamBuf
// Description : Used by LineStream to implement an ostream that
//               writes to a memory buffer, whose contents can be
//               continuously extracted as a sequence of lines of
//               text.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LineStreamBuf : public streambuf {
public:
  LineStreamBuf();
  virtual ~LineStreamBuf();

  INLINE bool is_text_available() const;
  string get_line();
  INLINE bool has_newline() const;

protected:
  virtual int overflow(int c);
  virtual int sync();

private:
  INLINE void write_chars(const char *start, int length);

  string _data;
  bool _has_newline;
};

#include "lineStreamBuf.I"

#endif
