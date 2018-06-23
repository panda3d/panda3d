/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file identityStreamBuf.h
 * @author drose
 * @date 2002-10-09
 */

#ifndef IDENTITYSTREAMBUF_H
#define IDENTITYSTREAMBUF_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

#include "bioStreamPtr.h"
#include "pointerTo.h"
#include "socketStream.h"

class HTTPChannel;

/**
 * The streambuf object that implements IIdentityStream.
 */
class EXPCL_PANDA_DOWNLOADER IdentityStreamBuf : public std::streambuf {
public:
  IdentityStreamBuf();
  virtual ~IdentityStreamBuf();

  void open_read(BioStreamPtr *source, HTTPChannel *doc,
                 bool has_content_length, size_t content_length);
  void close_read();

  INLINE bool is_closed() const;
  INLINE ISocketStream::ReadState get_read_state() const;

protected:
  virtual int underflow();

private:
  size_t read_chars(char *start, size_t length);

  PT(BioStreamPtr) _source;
  bool _has_content_length;
  size_t _bytes_remaining;
  bool _wanted_nonblocking;
  ISocketStream::ReadState _read_state;
  char *_buffer;

  friend class IIdentityStream;
};

#include "identityStreamBuf.I"

#endif  // HAVE_OPENSSL

#endif
