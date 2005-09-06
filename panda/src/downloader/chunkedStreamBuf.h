// Filename: chunkedStreamBuf.h
// Created by:  drose (25Sep02)
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

#ifndef CHUNKEDSTREAMBUF_H
#define CHUNKEDSTREAMBUF_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_SSL

#include "httpChannel.h"
#include "bioStreamPtr.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : ChunkedStreamBuf
// Description : The streambuf object that implements
//               IChunkedStream.
////////////////////////////////////////////////////////////////////
// No need to export from DLL.
class ChunkedStreamBuf : public streambuf {
public:
  ChunkedStreamBuf();
  virtual ~ChunkedStreamBuf();

  void open_read(BioStreamPtr *source, HTTPChannel *doc);
  void close_read();

protected:
  virtual int underflow();

private:
  size_t read_chars(char *start, size_t length);
  bool http_getline(string &str);

  PT(BioStreamPtr) _source;
  size_t _chunk_remaining;
  bool _done;
  string _working_getline;

  PT(HTTPChannel) _doc;
  int _read_index;

  friend class IChunkedStream;
};

#endif  // HAVE_SSL

#endif
