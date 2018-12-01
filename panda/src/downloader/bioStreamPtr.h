/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bioStreamPtr.h
 * @author drose
 * @date 2002-10-15
 */

#ifndef BIOSTREAMPTR_H
#define BIOSTREAMPTR_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

#include "bioStream.h"
#include "referenceCount.h"

/**
 * A wrapper around an BioStream object to make a reference-counting pointer
 * to it.
 */
class EXPCL_PANDA_DOWNLOADER BioStreamPtr : public ReferenceCount {
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
