// Filename: bioStreamBuf.h
// Created by:  drose (25Sep02)
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

#ifndef BIOSTREAMBUF_H
#define BIOSTREAMBUF_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL
#define OPENSSL_NO_KRB5

#include "bioPtr.h"
#include "pointerTo.h"
#include "openSSLWrapper.h"  // must be included before any other openssl.
#include "openssl/ssl.h"

////////////////////////////////////////////////////////////////////
//       Class : BioStreamBuf
// Description : The streambuf object that implements
//               IBioStream.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS BioStreamBuf : public streambuf {
public:
  BioStreamBuf();
  virtual ~BioStreamBuf();

  void open(BioPtr *source);
  void close();

protected:
  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();

private:
  size_t write_chars(const char *start, size_t length);

  PT(BioPtr) _source;
  bool _read_open;
  bool _write_open;
  char *_buffer;

  friend class IBioStream;
  friend class OBioStream;
  friend class BioStream;
};

#endif  // HAVE_OPENSSL

#endif
