/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file softEggGroupUserData.I
 * @author masad
 * @date 2003-09-25
 */

////////////////////////////////////////////////////////////////////
//     Function: SoftEggGroupUserData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SoftEggGroupUserData::
SoftEggGroupUserData() {
  _vertex_color = false;
  _double_sided = false;
}


////////////////////////////////////////////////////////////////////
//     Function: SoftEggGroupUserData::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SoftEggGroupUserData::
SoftEggGroupUserData(const SoftEggGroupUserData &copy) : 
  EggUserData(copy),
  _vertex_color(copy._vertex_color),
  _double_sided(copy._double_sided)
{
}


////////////////////////////////////////////////////////////////////
//     Function: SoftEggGroupUserData::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void SoftEggGroupUserData::
operator = (const SoftEggGroupUserData &copy) {
  EggUserData::operator = (copy);
  _vertex_color = copy._vertex_color;
  _double_sided = copy._double_sided;
}
