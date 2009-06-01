// Filename: physxBox.I
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
//  Description :
////////////////////////////////////////////////////////////////////
INLINE bool PhysxBox::
is_valid() const {
  nassertr(nBox != NULL, false);

  return nBox->isValid();
}

////////////////////////////////////////////////////////////////////
//     Function : rotate
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBox::
rotate(const LMatrix4f & mtx, PhysxBox & obb) const {
  nassertv(nBox != NULL);

  nBox->rotate(PhysxManager::lMatrix4_to_nxMat34(mtx), *(obb.nBox));
}

////////////////////////////////////////////////////////////////////
//     Function : set_empty
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBox::
set_empty() {
  nassertv(nBox != NULL);

  nBox->setEmpty();
}

