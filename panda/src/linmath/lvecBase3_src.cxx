// Filename: lvecBase3_src.cxx
// Created by:  drose (08Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
