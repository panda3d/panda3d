/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file chunkedStream.h
 * @author drose
 * @date 2002-09-25
 */

#ifndef CHUNKEDSTREAM_H
#define CHUNKEDSTREAM_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

#include "socketStream.h"
#include "chunkedStreamBuf.h"

class HTTPChannel;
class BioStreamPtr;

/**
 * An input stream object that reads data from a source istream, but
 * automatically decodes the "chunked" transfer-coding specified by an HTTP
 * server.
 *
 * Seeking is not supported.  No need to export from DLL.
 */
class IChunkedStream : public ISocketStream {
public:
  INLINE IChunkedStream();
  INLINE IChunkedStream(BioStreamPtr *source, HTTPChannel *doc);

  INLINE IChunkedStream &open(BioStreamPtr *source, HTTPChannel *doc);
  virtual ~IChunkedStream();

  virtual bool is_closed();
  virtual void close();
  virtual ReadState get_read_state();

private:
  ChunkedStreamBuf _buf;
};

#include "chunkedStream.I"

#endif  // HAVE_OPENSSL

#endif
