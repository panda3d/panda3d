/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bioStream.h
 * @author drose
 * @date 2002-09-25
 */

#ifndef BIOSTREAM_H
#define BIOSTREAM_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

#include "socketStream.h"
#include "bioStreamBuf.h"

/**
 * An input stream object that reads data from an OpenSSL BIO object.  This is
 * used by the HTTPClient and HTTPChannel classes to provide a C++ interface
 * to OpenSSL.
 *
 * Seeking is not supported.
 */
class EXPCL_PANDA_DOWNLOADER IBioStream : public ISocketStream {
public:
  INLINE IBioStream();
  INLINE IBioStream(BioPtr *source);

  INLINE IBioStream &open(BioPtr *source);

  virtual bool is_closed();
  virtual void close();
  virtual ReadState get_read_state();

private:
  BioStreamBuf _buf;
};

/**
 * An output stream object that writes data to an OpenSSL BIO object.  This is
 * used by the HTTPClient and HTTPChannel classes to provide a C++ interface
 * to OpenSSL.
 *
 * Seeking is not supported.
 */
class EXPCL_PANDA_DOWNLOADER OBioStream : public OSocketStream {
public:
  INLINE OBioStream();
  INLINE OBioStream(BioPtr *source);

  INLINE OBioStream &open(BioPtr *source);

  virtual bool is_closed();
  virtual void close();

private:
  BioStreamBuf _buf;
};

/**
 * A bi-directional stream object that reads and writes data to an OpenSSL BIO
 * object.
 */
class EXPCL_PANDA_DOWNLOADER BioStream : public SocketStream {
public:
  INLINE BioStream();
  INLINE BioStream(BioPtr *source);

  INLINE BioStream &open(BioPtr *source);

  virtual bool is_closed();
  virtual void close();

private:
  BioStreamBuf _buf;
};

#include "bioStream.I"

#endif  // HAVE_OPENSSL


#endif
