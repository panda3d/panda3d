// Filename: identityStreamBuf.h
// Created by:  drose (09Oct02)
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

#ifndef IDENTITYSTREAMBUF_H
#define IDENTITYSTREAMBUF_H

#include "pandabase.h"
#include "httpDocument.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : IdentityStreamBuf
// Description : The streambuf object that implements
//               IIdentityStream.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS IdentityStreamBuf : public streambuf {
public:
  IdentityStreamBuf();
  virtual ~IdentityStreamBuf();

  void open_read(istream *source, bool owns_source, HTTPDocument *doc,
                 size_t content_length);
  void close_read();

protected:
  virtual int underflow(void);

private:
  size_t read_chars(char *start, size_t length);

  istream *_source;
  bool _owns_source;
  size_t _bytes_remaining;

  PT(HTTPDocument) _doc;
  int _read_index;
};

#endif
