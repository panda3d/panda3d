// Filename: p3dReferenceCount.I
// Created by:  drose (09Jul09)
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
//     Function: P3DReferenceCount::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline P3DReferenceCount::
P3DReferenceCount() {
  _ref_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DReferenceCount::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline P3DReferenceCount::
~P3DReferenceCount() {
  assert(_ref_count == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DReferenceCount::ref
//       Access: Public
//  Description: Explicitly increments the reference count.
////////////////////////////////////////////////////////////////////
inline void P3DReferenceCount::
ref() const {
  ++((P3DReferenceCount *)this)->_ref_count;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DReferenceCount::unref
//       Access: Public
//  Description: Explicitly decrements the reference count.  Usually,
//               you should call p3d_unref_delete() instead.
//
//               The return value is true if the new reference count
//               is nonzero, false if it is zero.
////////////////////////////////////////////////////////////////////
inline bool P3DReferenceCount::
unref() const {
  return --(((P3DReferenceCount *)this)->_ref_count) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DReferenceCount::get_ref_count
//       Access: Public
//  Description: Returns the current reference count.
////////////////////////////////////////////////////////////////////
inline int P3DReferenceCount::
get_ref_count() const {
  return _ref_count;
}

////////////////////////////////////////////////////////////////////
//     Function: p3d_unref_delete
//  Description: This global helper function will unref the given
//               P3DReferenceCount object, and if the reference count
//               reaches zero, automatically delete it.
////////////////////////////////////////////////////////////////////
template<class RefCountType>
inline void
p3d_unref_delete(RefCountType *ptr) {
  if (!ptr->unref()) {
    delete ptr;
  } 
}
