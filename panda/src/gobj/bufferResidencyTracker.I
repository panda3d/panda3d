// Filename: bufferResidencyTracker.I
// Created by:  drose (16Mar06)
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
//     Function: BufferResidencyTracker::get_inactive_nonresident
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BufferContextChain &BufferResidencyTracker::
get_inactive_nonresident() {
  return _chains[S_inactive_nonresident];
}

////////////////////////////////////////////////////////////////////
//     Function: BufferResidencyTracker::get_active_nonresident
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BufferContextChain &BufferResidencyTracker::
get_active_nonresident() {
  return _chains[S_active_nonresident];
}

////////////////////////////////////////////////////////////////////
//     Function: BufferResidencyTracker::get_inactive_resident
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BufferContextChain &BufferResidencyTracker::
get_inactive_resident() {
  return _chains[S_inactive_resident];
}

////////////////////////////////////////////////////////////////////
//     Function: BufferResidencyTracker::get_active_resident
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BufferContextChain &BufferResidencyTracker::
get_active_resident() {
  return _chains[S_active_resident];
}
