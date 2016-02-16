/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxUtilLib.I
 * @author enn0x
 * @date 2009-11-01
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxUtilLib::
PhysxUtilLib() {

  _ptr = NxGetUtilLib();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxUtilLib::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxUtilLib::
~PhysxUtilLib() {

  _ptr = NULL;
}

