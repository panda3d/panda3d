/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxBodyDesc.I
 * @author enn0x
 * @date 2009-09-05
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxBodyDesc::
PhysxBodyDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxBodyDesc::
~PhysxBodyDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxBodyDesc::
set_to_default() {

  _desc.setToDefault();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxBodyDesc::
is_valid() const {

  return _desc.isValid();
}

