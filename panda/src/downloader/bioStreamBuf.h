// Filename: bioStreamBuf.h
// Created by:  drose (25Sep02)
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

#ifndef BIOSTREAMBUF_H
#define BIOSTREAMBUF_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_SSL

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

  void open_read(BIO *source, bool owns_source);
  void close_read();

protected:
  virtual int underflow(void);

private:
  BIO *_source;
  bool _owns_source;
};

#endif  // HAVE_SSL

#endif
