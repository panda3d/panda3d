// Filename: identityStream.h
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

#ifndef IDENTITYSTREAM_H
#define IDENTITYSTREAM_H

#include "pandabase.h"

#include "identityStreamBuf.h"

class HTTPDocument;

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
//               updates the HTTPDocument when the stream is
//               completely read.
////////////////////////////////////////////////////////////////////
// No need to export from DLL.
class IIdentityStream : public istream {
public:
  INLINE IIdentityStream();
  INLINE IIdentityStream(istream *source, bool owns_source, 
                         HTTPDocument *doc, size_t content_length);

  INLINE IIdentityStream &open(istream *source, bool owns_source, 
                              HTTPDocument *doc, size_t content_length);
  INLINE IIdentityStream &close();

private:
  IdentityStreamBuf _buf;
};

#include "identityStream.I"

#endif


