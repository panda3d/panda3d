// Filename: mutexNsprImpl.I
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
//     Function: MutexNsprImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexNsprImpl::
MutexNsprImpl() {
  _lock = PR_NewLock();
  nassertv(_lock != (PRLock *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexNsprImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexNsprImpl::
~MutexNsprImpl() {
  PR_DestroyLock(_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexNsprImpl::lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexNsprImpl::
lock() {
  PR_Lock(_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexNsprImpl::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexNsprImpl::
release() {
  int status = PR_Unlock(_lock);
  nassertv(status == PR_SUCCESS);
}
