// Filename: subStreamBuf.h
// Created by:  drose (02Aug02)
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

#ifndef SUBSTREAMBUF_H
#define SUBSTREAMBUF_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : SubStreamBuf
// Description : The streambuf object that implements ISubStream.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS SubStreamBuf : public streambuf {
public:
  SubStreamBuf();
  virtual ~SubStreamBuf();

  void open(istream *source, streampos start, streampos end);
  void close();

  virtual streampos seekoff(streamoff off, ios_seekdir dir, ios_openmode mode);
  virtual streampos seekpos(streampos pos, ios_openmode mode);

protected:
  virtual int overflow(int c);
  virtual int sync(void);
  virtual int underflow(void);

private:
  istream *_source;
  streampos _start;
  streampos _end;
  streampos _cur;
  size_t _unused;
};

#endif
