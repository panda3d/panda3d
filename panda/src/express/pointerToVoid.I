// Filename: pointerToVoid.I
// Created by:  drose (27Sep04)
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
//     Function: PointerToVoid::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PointerToVoid::
PointerToVoid() {
  _void_ptr = (void *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToVoid::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PointerToVoid::
~PointerToVoid() {
  nassertv(_void_ptr == (void *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToVoid::is_null
//       Access: Published
//  Description: Returns true if the PointerTo is a NULL pointer,
//               false otherwise.  (Direct comparison to a NULL
//               pointer also works.)
////////////////////////////////////////////////////////////////////
INLINE bool PointerToVoid::
is_null() const {
  return (_void_ptr == (void *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToVoid::get_hash
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE size_t PointerToVoid::
get_hash() const {
  return (size_t)_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToVoid::Less-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool PointerToVoid::
operator < (const void *other) const {
  return _void_ptr < other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToVoid::Less-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool PointerToVoid::
operator < (const PointerToVoid &other) const {
  return _void_ptr < other._void_ptr;
}

