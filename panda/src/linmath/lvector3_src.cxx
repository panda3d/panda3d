// Filename: lvector3_src.cxx
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

TypeHandle FLOATNAME(LVector3)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LVector3::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LVector3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase3)::init_type();
    register_type(_type_handle, FLOATNAME_STR(LVector3),
                  FLOATNAME(LVecBase3)::get_class_type());
  }
}


