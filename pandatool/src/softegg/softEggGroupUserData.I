// Filename: softEggGroupUserData.I
// Created by:  masad (25Sep03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2003, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


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
