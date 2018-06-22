/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file chunkedStreamBuf.h
 * @author drose
 * @date 2002-09-25
 */

#ifndef CHUNKEDSTREAMBUF_H
#define CHUNKEDSTREAMBUF_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

#include "httpChannel.h"
#include "bioStreamPtr.h"
#include "pointerTo.h"

/**
 * The streambuf object that implements IChunkedStream.
 */
class ChunkedStreamBuf : public std::streambuf {
  // No need to export from DLL.
public:
  ChunkedStreamBuf();
  virtual ~ChunkedStreamBuf();

  void open_read(BioStreamPtr *source, HTTPChannel *doc);
  void close_read();

  INLINE bool is_closed() const;
  INLINE ISocketStream::ReadState get_read_state() const;

protected:
  virtual int underflow();

private:
  size_t read_chars(char *start, size_t length);
  bool http_getline(std::string &str);

  PT(BioStreamPtr) _source;
  size_t _chunk_remaining;
  bool _done;
  bool _wanted_nonblocking;
  std::string _working_getline;
  ISocketStream::ReadState _read_state;

  PT(HTTPChannel) _doc;
  int _read_index;
  char *_buffer;

  friend class IChunkedStream;
};

#include "chunkedStreamBuf.I"

#endif  // HAVE_OPENSSL

#endif
