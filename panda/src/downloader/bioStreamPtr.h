// Filename: bioStreamPtr.h
// Created by:  drose (15Oct02)
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

#ifndef BIOSTREAMPTR_H
#define BIOSTREAMPTR_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_SSL
#define OPENSSL_NO_KRB5

#include "bioStream.h"
#include "referenceCount.h"
#include <openssl/ssl.h>

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

  bool connect() const;
  
private:
  BioStream *_stream;
};

#include "bioStreamPtr.I"

#endif  // HAVE_SSL


#endif


