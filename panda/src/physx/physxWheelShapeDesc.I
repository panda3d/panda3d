/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxWheelShapeDesc.I
 * @author enn0x
 * @date 2009-11-09
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxWheelShapeDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxWheelShapeDesc::
PhysxWheelShapeDesc() : PhysxShapeDesc() {

  set_to_default();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxWheelShapeDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxWheelShapeDesc::
~PhysxWheelShapeDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxWheelShapeDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxWheelShapeDesc::
is_valid() const {

  return _desc.isValid();
}

