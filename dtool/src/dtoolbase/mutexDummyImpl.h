// Filename: mutexDummyImpl.h
// Created by:  drose (08Aug02)
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

#ifndef MUTEXDUMMYIMPL_H
#define MUTEXDUMMYIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

////////////////////////////////////////////////////////////////////
//       Class : MutexDummyImpl
// Description : A fake mutex implementation for single-threaded
//               applications that don't need any synchronization
//               control.  This does nothing at all.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL MutexDummyImpl {
public:
  INLINE MutexDummyImpl();
  INLINE ~MutexDummyImpl();

  INLINE void acquire();
  INLINE bool try_acquire();
  INLINE void release();
};

#include "mutexDummyImpl.I"

#endif
