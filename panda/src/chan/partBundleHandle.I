// Filename: partBundleHandle.I
// Created by:  drose (01Oct07)
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


////////////////////////////////////////////////////////////////////
//     Function: PartBundleHandle::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PartBundleHandle::
PartBundleHandle(PartBundle *bundle) :
  _bundle(bundle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleHandle::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PartBundleHandle::
~PartBundleHandle() {
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleHandle::get_bundle
//       Access: Published
//  Description: Returns the actual PartBundle embedded within the
//               handle.
////////////////////////////////////////////////////////////////////
INLINE PartBundle *PartBundleHandle::
get_bundle() {
  return _bundle;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleHandle::set_bundle
//       Access: Published
//  Description: Changes the actual PartBundle embedded within the
//               handle.
////////////////////////////////////////////////////////////////////
INLINE void PartBundleHandle::
set_bundle(PartBundle *bundle) {
  _bundle = bundle;
}
