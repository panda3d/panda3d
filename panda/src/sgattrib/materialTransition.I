// Filename: materialTransition.I
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
//     Function: MaterialTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MaterialTransition::
MaterialTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MaterialTransition::
MaterialTransition(const Material *material) :
  OnOffTransition(TD_on),
  _value(material)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::off
//       Access: Public
//  Description: This named constructor returns a MaterialTransition
//               that disables materials.
////////////////////////////////////////////////////////////////////
INLINE MaterialTransition MaterialTransition::
off() {
  MaterialTransition result;
  result.set_off();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::set_on
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void MaterialTransition::
set_on(const Material *material) {
  nassertv(material != (const Material *)NULL);
  _value = material;
  OnOffTransition::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::get_material
//       Access: Public
//  Description: Returns the material that the MaterialTransition
//               represents.  It is only valid to call this if is_on()
//               has returned true.
////////////////////////////////////////////////////////////////////
INLINE const Material *MaterialTransition::
get_material() const {
  nassertr(is_on(), NULL);
  return _value;
}
