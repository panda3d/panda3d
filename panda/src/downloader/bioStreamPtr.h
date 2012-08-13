// Filename: bioStreamPtr.h
// Created by:  drose (15Oct02)
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

#ifndef BIOSTREAMPTR_H
#define BIOSTREAMPTR_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL
#define OPENSSL_NO_KRB5

#include "bioStream.h"
#include "referenceCount.h"
#include "openSSLWrapper.h"  // must be included before any other openssl.
#include "openssl/ssl.h"

////////////////////////////////////////////////////////////////////
//       Class : BioStreamPtr
// Description : A wrapper around an BioStream object to make a
//               reference-counting pointer to it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS BioStreamPtr : public ReferenceCount {
public:
  INLINE BioStreamPtr(BioStream *stream);
  virtual ~BioStreamPtr();

  INLINE BioStream &operator *() const;
  INLINE BioStream *operator -> () const;
  INLINE operator BioStream * () const;

  INLINE void set_stream(BioStream *stream);
  INLINE BioStream *get_stream() const;

private:
  BioStream *_stream;
};

#include "bioStreamPtr.I"

#endif  // HAVE_OPENSSL


#endif


