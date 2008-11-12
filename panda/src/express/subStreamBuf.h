// Filename: subStreamBuf.h
// Created by:  drose (02Aug02)
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

#ifndef SUBSTREAMBUF_H
#define SUBSTREAMBUF_H

#include "pandabase.h"
#include "streamWrapper.h"

////////////////////////////////////////////////////////////////////
//       Class : SubStreamBuf
// Description : The streambuf object that implements ISubStream.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS SubStreamBuf : public streambuf {
public:
  SubStreamBuf();
  virtual ~SubStreamBuf();

  void open(IStreamWrapper *source, streampos start, streampos end);
  void close();

  virtual streampos seekoff(streamoff off, ios_seekdir dir, ios_openmode mode);
  virtual streampos seekpos(streampos pos, ios_openmode mode);

protected:
  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();

private:
  IStreamWrapper *_source;
  streampos _start;
  streampos _end;
  streampos _cur;
  size_t _unused;
  char *_buffer;
};

#endif
