// Filename: lvecBase3_src.cxx
// Created by:  drose (08Mar00)
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


TypeHandle FLOATNAME(LVecBase3)::_type_handle;

const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_zero =
  FLOATNAME(LVecBase3)(0.0f, 0.0f, 0.0f);
const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_unit_x =
  FLOATNAME(LVecBase3)(1.0f, 0.0f, 0.0f);
const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_unit_y =
  FLOATNAME(LVecBase3)(0.0f, 1.0f, 0.0f);
const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_unit_z =
  FLOATNAME(LVecBase3)(0.0f, 0.0f, 1.0f);

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LVecBase3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    string name = "LVecBase3";
    name += FLOATTOKEN;
    register_type(_type_handle, name);
  }
}

