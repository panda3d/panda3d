// Filename: mutexDummyImpl.I
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


////////////////////////////////////////////////////////////////////
//     Function: MutexDummyImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexDummyImpl::
MutexDummyImpl() {
#ifndef NDEBUG
  _lock_count = 0;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDummyImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexDummyImpl::
~MutexDummyImpl() {
  nassertv(_lock_count == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDummyImpl::lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexDummyImpl::
lock() {
#ifndef NDEBUG
  _lock_count++;
  nassertv(_lock_count == 1);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDummyImpl::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexDummyImpl::
release() {
#ifndef NDEBUG
  _lock_count--;
  nassertv(_lock_count == 0);
#endif
}
