// Filename: lpoint4_src.cxx
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

TypeHandle FLOATNAME(LPoint4)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////

void FLOATNAME(LPoint4)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase4)::init_type();
    string name = "LPoint4";
    name += FLOATTOKEN;
    register_type(_type_handle, name,
                  FLOATNAME(LVecBase4)::get_class_type());
  }
}


