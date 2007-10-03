// Filename: partBundleHandle.I
// Created by:  drose (01Oct07)
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
