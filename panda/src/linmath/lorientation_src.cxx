// Filename: lorientation_src.cxx
// Created by:  frang, charles (23Jun00)
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

TypeHandle FLOATNAME(LOrientation)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LOrientation)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LQuaternion)::init_type();
    register_type(_type_handle, FLOATNAME_STR(LOrientation),
                  FLOATNAME(LQuaternion)::get_class_type());
  }
}
