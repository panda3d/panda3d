// Filename: bioStreamBuf.h
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

#ifndef BIOSTREAMBUF_H
#define BIOSTREAMBUF_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_SSL
#define OPENSSL_NO_KRB5

#include "bioPtr.h"
#include "pointerTo.h"
#include <openssl/ssl.h>

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
  virtual int sync(void);
  virtual int underflow(void);

private:
  size_t write_chars(const char *start, size_t length);

  PT(BioPtr) _source;
  bool _is_closed;

  friend class IBioStream;
  friend class OBioStream;
  friend class BioStream;
};

#endif  // HAVE_SSL

#endif
