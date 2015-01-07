// Filename: pointerToVoid.I
// Created by:  drose (27Sep04)
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
//     Function: PointerToVoid::Copy Constructor
//       Access: Private
//  Description: Don't use this constructor.
////////////////////////////////////////////////////////////////////
INLINE PointerToVoid::
PointerToVoid(const PointerToVoid &) {
  nassertv(false);
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

////////////////////////////////////////////////////////////////////
//     Function: PointerToVoid::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool PointerToVoid::
operator == (const PointerToVoid &other) const {
  return _void_ptr == other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToVoid::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool PointerToVoid::
operator != (const PointerToVoid &other) const {
  return _void_ptr != other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToVoid::swap
//       Access: Public
//  Description: Swaps the contents of this PointerTo with the other,
//               without touching the reference counts.
//
//               For internal use only.  Use the global swap()
//               function instead.
////////////////////////////////////////////////////////////////////
INLINE void PointerToVoid::
swap(PointerToVoid &other) NOEXCEPT {
  AtomicAdjust::Pointer temp = _void_ptr;
  _void_ptr = other._void_ptr;
  other._void_ptr = temp;
}
