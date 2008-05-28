// Filename: lrotation_src.cxx
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

TypeHandle FLOATNAME(LRotation)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LRotation::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LRotation)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LQuaternion)::init_type();
    // Format a string to describe the type.
    string name = "LRotation";
    name += FLOATTOKEN;
    register_type(_type_handle, name,
                  FLOATNAME(LQuaternion)::get_class_type());
  }
}

