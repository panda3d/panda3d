// Filename: materialAttrib.I
// Created by:  drose (04Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
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
//     Function: MaterialAttrib::Constructor
//       Access: Private
//  Description: Use MaterialAttrib::make() to construct a new
//               MaterialAttrib object.
////////////////////////////////////////////////////////////////////
INLINE MaterialAttrib::
MaterialAttrib() {
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::is_off
//       Access: Published
//  Description: Returns true if the MaterialAttrib is an 'off'
//               MaterialAttrib, indicating that it should disable
//               texturing.
////////////////////////////////////////////////////////////////////
INLINE bool MaterialAttrib::
is_off() const {
  return _material == (const Material *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::get_material
//       Access: Published
//  Description: If the MaterialAttrib is not an 'off' MaterialAttrib,
//               returns the material that is associated.  Otherwise,
//               return NULL.
////////////////////////////////////////////////////////////////////
INLINE const Material *MaterialAttrib::
get_material() const {
  return _material;
}
