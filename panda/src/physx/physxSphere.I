/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSphere.I
 * @author enn0x
 * @date 2009-10-31
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSphere::
PhysxSphere() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSphere::
PhysxSphere(const PhysxSphere &sphere) {

  _sphere = sphere._sphere;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSphere::
~PhysxSphere() {

}

