// Filename: lrotation_src.cxx
// Created by:  frang, charles (23Jun00)
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

