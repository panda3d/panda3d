/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxPlane.I
 * @author enn0x
 * @date 2009-10-31
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxPlane::
PhysxPlane() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxPlane::
PhysxPlane(const PhysxPlane &plane) {

  _plane = plane._plane;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxPlane::
~PhysxPlane() {

}

