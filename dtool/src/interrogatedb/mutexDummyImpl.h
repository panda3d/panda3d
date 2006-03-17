// Filename: mutexDummyImpl.h
// Created by:  drose (08Aug02)
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

#ifndef MUTEXDUMMYIMPL_H
#define MUTEXDUMMYIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_DUMMY_IMPL

#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//       Class : MutexDummyImpl
// Description : A fake mutex implementation for single-threaded
//               applications that don't need any synchronization
//               control.  This does nothing at all.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG MutexDummyImpl {
public:
  INLINE MutexDummyImpl();
  INLINE ~MutexDummyImpl();

  INLINE void lock();
  INLINE void release();
};

#include "mutexDummyImpl.I"

#endif  // THREAD_DUMMY_IMPL

#endif
