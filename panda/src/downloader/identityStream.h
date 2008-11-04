// Filename: identityStream.h
// Created by:  drose (09Oct02)
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

#ifndef IDENTITYSTREAM_H
#define IDENTITYSTREAM_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

#include "socketStream.h"
#include "identityStreamBuf.h"

class HTTPChannel;
class BioStreamPtr;

////////////////////////////////////////////////////////////////////
//       Class : IIdentityStream
// Description : An input stream object that reads data from a source
//               istream, but automatically decodes the "identity"
//               transfer-coding specified by an HTTP server.
//
//               In practice, this just means it reads from the sub
//               stream (like a SubStreamBuf) up to but not past the
//               specified content-length.  (If the content-length was
//               unspecified, this class cannot be used.)  It also
//               updates the HTTPChannel when the stream is
//               completely read.
////////////////////////////////////////////////////////////////////
// No need to export from DLL.
class IIdentityStream : public ISocketStream {
public:
  INLINE IIdentityStream();
  INLINE IIdentityStream(BioStreamPtr *source, HTTPChannel *doc,
                         bool has_content_length, size_t content_length);

  INLINE IIdentityStream &open(BioStreamPtr *source, HTTPChannel *doc,
                               bool has_content_length, size_t content_length);
  virtual ~IIdentityStream();

  virtual bool is_closed();
  virtual void close();
  virtual ReadState get_read_state();

private:
  IdentityStreamBuf _buf;
};

#include "identityStream.I"

#endif  // HAVE_OPENSSL

#endif


