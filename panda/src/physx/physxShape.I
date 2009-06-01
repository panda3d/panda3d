// Filename: physxShape.I
// Created by:  pratt (Apr 7, 2006)
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
//     Function : is_valid
//       Access : Published
//  Description : Returns true if the NxShape associated with this
//                is not NULL.  The NxShape will be NULL if the
//                Actor that this shape is a part of has been
//                released.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxShape::
is_valid() {
  return nShape != NULL;
}

////////////////////////////////////////////////////////////////////
//     Function : get_app_data
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void * PhysxShape::
get_app_data() const {
  nassertr(nShape != NULL, NULL);

  return nShape->appData;
}

////////////////////////////////////////////////////////////////////
//     Function : set_app_data
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxShape::
set_app_data(void * value) {
  nassertv(nShape != NULL);

  nShape->appData = value;
}

