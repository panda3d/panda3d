// Filename: memoryInfo.I
// Created by:  drose (04Jun01)
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
//     Function: MemoryInfo::get_void_ptr
//       Access: Public
//  Description: Returns the data pointer as a void pointer.  This
//               should always be non-NULL.
////////////////////////////////////////////////////////////////////
void *MemoryInfo::get_void_ptr() const {
  return _void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryInfo::get_ref_ptr
//       Access: Public
//  Description: Returns the data pointer as a ReferenceCount pointer.
//               This may be NULL if the data pointer does not
//               represent a ReferenceCount object.
////////////////////////////////////////////////////////////////////
ReferenceCount *MemoryInfo::get_ref_ptr() const {
  return _ref_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryInfo::get_typed_ptr
//       Access: Public
//  Description: Returns the data pointer as a TypedObject pointer.
//               This may be NULL if the data pointer does not
//               represent a pointer to a TypedObject.
////////////////////////////////////////////////////////////////////
TypedObject *MemoryInfo::get_typed_ptr() const {
  return _typed_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryInfo::is_size_known
//       Access: Public
//  Description: Returns true if the size of the memory block
//               referenced by this pointer is known.  Most pointers'
//               sizes should be known, but some may not be.
////////////////////////////////////////////////////////////////////
bool MemoryInfo::is_size_known() const {
  return (_flags & F_size_known) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryInfo::get_size
//       Access: Public
//  Description: Returns the size in bytes of the memory block
//               referenced by this pointer, if it is known.  Returns
//               zero if the size is not known.
////////////////////////////////////////////////////////////////////
size_t MemoryInfo::get_size() const {
  return _size;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryInfo::get_time
//       Access: Public
//  Description: Returns the time in seconds (based on the
//               GlobalClock) at which the pointer was allocated.
////////////////////////////////////////////////////////////////////
double MemoryInfo::get_time() const {
  return _time;
}

