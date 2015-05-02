// Filename: lpoint3_src.cxx
// Created by:  drose (25Sep99)
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

TypeHandle FLOATNAME(LPoint3)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LPoint3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase3)::init_type();
    register_type(_type_handle, FLOATNAME_STR(LPoint3),
                  FLOATNAME(LVecBase3)::get_class_type());
  }
}

