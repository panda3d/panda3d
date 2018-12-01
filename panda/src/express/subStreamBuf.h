/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subStreamBuf.h
 * @author drose
 * @date 2002-08-02
 */

#ifndef SUBSTREAMBUF_H
#define SUBSTREAMBUF_H

#include "pandabase.h"
#include "streamWrapper.h"

/**
 * The streambuf object that implements ISubStream.
 */
class EXPCL_PANDA_EXPRESS SubStreamBuf : public std::streambuf {
public:
  SubStreamBuf();
  SubStreamBuf(const SubStreamBuf &copy) = delete;
  virtual ~SubStreamBuf();

  void open(IStreamWrapper *source, OStreamWrapper *dest, std::streampos start, std::streampos end, bool append);
  void close();

  virtual std::streampos seekoff(std::streamoff off, ios_seekdir dir, ios_openmode which);
  virtual std::streampos seekpos(std::streampos pos, ios_openmode which);

protected:
  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();

private:
  IStreamWrapper *_source;
  OStreamWrapper *_dest;
  std::streampos _start;
  std::streampos _end;
  bool _append;
  std::streampos _gpos;
  std::streampos _ppos;
  char *_buffer;
};

#endif
