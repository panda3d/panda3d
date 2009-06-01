// Filename: physxSceneStats2.cxx
// Created by:  pratt (Jun 20, 2006)
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

#ifdef HAVE_PHYSX

#include "physxSceneStats2.h"


////////////////////////////////////////////////////////////////////
//     Function : PhysxSceneStats2
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxSceneStats2::
PhysxSceneStats2(const NxSceneStats2 *stats2) {
  nSceneStats2 = stats2;
}

////////////////////////////////////////////////////////////////////
//     Function : ~PhysxSceneStats2
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxSceneStats2::
~PhysxSceneStats2() {
}

#endif // HAVE_PHYSX
