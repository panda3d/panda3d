// Filename: socketStream.h
// Created by:  drose (15Oct02)
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

#ifndef SOCKETSTREAM_H
#define SOCKETSTREAM_H

#include "pandabase.h"

// At the present, this module is not compiled if OpenSSL is not
// available, since the only current use for it is to implement
// OpenSSL-defined constructs (like ISocketStream).

#ifdef HAVE_SSL

////////////////////////////////////////////////////////////////////
//       Class : ISocketStream
// Description : This is a base class for istreams implemented in
//               Panda that read from a (possibly non-blocking)
//               socket.  It adds is_closed(), which can be called
//               after an eof condition to check whether the socket
//               has been closed, or whether more data may be
//               available later.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ISocketStream : public istream {
public:
  INLINE ISocketStream(streambuf *buf);

PUBLISHED:
  virtual bool is_closed() = 0;
};

#include "socketStream.I"

#endif  // HAVE_SSL


#endif


