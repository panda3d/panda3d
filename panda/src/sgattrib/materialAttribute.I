// Filename: materialAttribute.I
// Created by:  drose (22Mar00)
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
//     Function: MaterialAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MaterialAttribute::
MaterialAttribute() {
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::set_on
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void MaterialAttribute::
set_on(const Material *material) {
  nassertv(material != (const Material *)NULL);
  _value = material;
  OnOffAttribute::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::get_material
//       Access: Public
//  Description: Returns the material that the MaterialAttribute
//               represents.  It is only valid to call this if is_on()
//               has returned true.
////////////////////////////////////////////////////////////////////
INLINE const Material *MaterialAttribute::
get_material() const {
  nassertr(is_on(), NULL);
  return _value;
}
