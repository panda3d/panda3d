// Filename: eggCoordinateSystem.I
// Created by:  drose (20Jan99)
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
//     Function: EggCoordinateSystem::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggCoordinateSystem::
EggCoordinateSystem(CoordinateSystem value) {
  _value = value;
}


////////////////////////////////////////////////////////////////////
//     Function: EggCoordinateSystem::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggCoordinateSystem::
EggCoordinateSystem(const EggCoordinateSystem &copy)
  : EggNode(copy), _value(copy._value) { }


////////////////////////////////////////////////////////////////////
//     Function: EggCoordinateSystem::set_value
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggCoordinateSystem::
set_value(CoordinateSystem value) {
  _value = value;
}


////////////////////////////////////////////////////////////////////
//     Function: EggCoordinateSystem::set_value
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem EggCoordinateSystem::
get_value() const {
  return _value;
}

