// Filename: eggObject.I
// Created by:  drose (10Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggObject::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggObject::
EggObject() {
}


////////////////////////////////////////////////////////////////////
//     Function: EggObject::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggObject::
EggObject(const EggObject &copy) : TypedReferenceCount(copy) {
}


////////////////////////////////////////////////////////////////////
//     Function: EggObject::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggObject &EggObject::
operator = (const EggObject &copy) {
  TypedReferenceCount::operator = (copy);
  return *this;
}
