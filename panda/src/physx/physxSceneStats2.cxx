/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSceneStats2.cxx
 * @author enn0x
 * @date 2009-10-20
 */

#include "physxSceneStats2.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneStats2::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PhysxSceneStats2::
PhysxSceneStats2(const NxSceneStats2 *ptr) {

  _ptr = ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneStats2::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PhysxSceneStats2::
~PhysxSceneStats2() {

}

