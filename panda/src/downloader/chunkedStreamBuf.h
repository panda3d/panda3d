// Filename: chunkedStreamBuf.h
// Created by:  drose (25Sep02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CHUNKEDSTREAMBUF_H
#define CHUNKEDSTREAMBUF_H

#include "pandabase.h"
#include "httpDocument.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : ChunkedStreamBuf
// Description : The streambuf object that implements
//               IChunkedStream.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ChunkedStreamBuf : public streambuf {
public:
  ChunkedStreamBuf();
  virtual ~ChunkedStreamBuf();

  void open_read(istream *source, bool owns_source, HTTPDocument *doc);
  void close_read();

protected:
  virtual int underflow(void);

private:
  size_t read_chars(char *start, size_t length);

  istream *_source;
  bool _owns_source;
  size_t _chunk_remaining;
  bool _done;

  PT(HTTPDocument) _doc;
};

#endif
