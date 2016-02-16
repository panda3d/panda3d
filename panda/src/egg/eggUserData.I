/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggUserData.I
 * @author drose
 * @date 2003-06-03
 */

////////////////////////////////////////////////////////////////////
//     Function: EggUserData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggUserData::
EggUserData() {
}


////////////////////////////////////////////////////////////////////
//     Function: EggUserData::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggUserData::
EggUserData(const EggUserData &copy) : TypedReferenceCount(copy) {
}


////////////////////////////////////////////////////////////////////
//     Function: EggUserData::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggUserData &EggUserData::
operator = (const EggUserData &copy) {
  TypedReferenceCount::operator = (copy);
  return *this;
}
