// Filename: mayaEggGroupUserData.I
// Created by:  drose (03Jun03)
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


////////////////////////////////////////////////////////////////////
//     Function: MayaEggGroupUserData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MayaEggGroupUserData::
MayaEggGroupUserData() {
  _vertex_color = false;
  _double_sided = false;
}


////////////////////////////////////////////////////////////////////
//     Function: MayaEggGroupUserData::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MayaEggGroupUserData::
MayaEggGroupUserData(const MayaEggGroupUserData &copy) : 
  EggUserData(copy),
  _vertex_color(copy._vertex_color),
  _double_sided(copy._double_sided)
{
}


////////////////////////////////////////////////////////////////////
//     Function: MayaEggGroupUserData::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void MayaEggGroupUserData::
operator = (const MayaEggGroupUserData &copy) {
  EggUserData::operator = (copy);
  _vertex_color = copy._vertex_color;
  _double_sided = copy._double_sided;
}
