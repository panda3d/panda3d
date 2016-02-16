/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSceneDesc.I
 * @author enn0x
 * @date 2009-09-05
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSceneDesc::
PhysxSceneDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSceneDesc::
~PhysxSceneDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxSceneDesc::
set_to_default() {

  _desc.setToDefault();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxSceneDesc::
is_valid() const {

  return _desc.isValid();
}

