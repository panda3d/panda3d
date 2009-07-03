// Filename: p3dObject.I
// Created by:  drose (30Jun09)
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
//     Function: P3DObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline P3DObject::
P3DObject() {
  _class = &_object_class;

  // The reference count is initially 1.
  _ref_count = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline P3DObject::
P3DObject(const P3DObject &copy) {
  assert(copy._class == &_object_class);
  _class = copy._class;
  _ref_count = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::ref
//       Access: Public
//  Description: Increments the reference count.
////////////////////////////////////////////////////////////////////
inline void P3DObject::
ref() const {
  ++(((P3DObject *)this)->_ref_count);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::unref
//       Access: Public
//  Description: Decrements the reference count, and returns the new
//               count.
////////////////////////////////////////////////////////////////////
inline int P3DObject::
unref() const {
  --(((P3DObject *)this)->_ref_count);
  return _ref_count;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::unref_delete
//       Access: Public, Static
//  Description: Decrements the reference count on the indicated
//               object, and deletes it if the reference count reaches
//               zero.
////////////////////////////////////////////////////////////////////
inline void P3DObject::
unref_delete(P3DObject *obj) {
  if (obj->unref() <= 0) {
    delete obj;
  }
}
