// Filename: lightAttrib.I
// Created by:  drose (26Mar02)
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


////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::Constructor
//       Access: Private
//  Description: Use LightAttrib::make() to construct a new
//               LightAttrib object.
////////////////////////////////////////////////////////////////////
INLINE LightAttrib::
LightAttrib() {
  _off_all_lights = false;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::Copy Constructor
//       Access: Private
//  Description: Use LightAttrib::make() to construct a new
//               LightAttrib object.  The copy constructor is only
//               defined to facilitate methods like add_on_light().
////////////////////////////////////////////////////////////////////
INLINE LightAttrib::
LightAttrib(const LightAttrib &copy) :
  _on_lights(copy._on_lights),
  _off_lights(copy._off_lights),
  _off_all_lights(copy._off_all_lights)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_num_on_lights
//       Access: Published
//  Description: Returns the number of lights that are turned on by
//               the attribute.
////////////////////////////////////////////////////////////////////
INLINE int LightAttrib::
get_num_on_lights() const {
  return _on_lights.size();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_on_light
//       Access: Published
//  Description: Returns the nth light turned on by the attribute,
//               sorted in render order.
////////////////////////////////////////////////////////////////////
INLINE Light *LightAttrib::
get_on_light(int n) const {
  nassertr(n >= 0 && n < (int)_on_lights.size(), (Light *)NULL);
  return _on_lights[n];
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::has_on_light
//       Access: Published
//  Description: Returns true if the indicated light is turned on by
//               the attrib, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool LightAttrib::
has_on_light(Light *light) const {
  return _on_lights.find(light) != _on_lights.end();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_num_off_lights
//       Access: Published
//  Description: Returns the number of lights that are turned off by
//               the attribute.
////////////////////////////////////////////////////////////////////
INLINE int LightAttrib::
get_num_off_lights() const {
  return _off_lights.size();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_off_light
//       Access: Published
//  Description: Returns the nth light turned off by the attribute,
//               sorted in arbitrary (pointer) order.
////////////////////////////////////////////////////////////////////
INLINE Light *LightAttrib::
get_off_light(int n) const {
  nassertr(n >= 0 && n < (int)_off_lights.size(), (Light *)NULL);
  return _off_lights[n];
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::has_off_light
//       Access: Published
//  Description: Returns true if the indicated light is turned off by
//               the attrib, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool LightAttrib::
has_off_light(Light *light) const {
  return _off_all_lights || _off_lights.find(light) != _off_lights.end();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::has_all_off
//       Access: Published
//  Description: Returns true if this attrib turns off all lights
//               (although it may also turn some on).
////////////////////////////////////////////////////////////////////
INLINE bool LightAttrib::
has_all_off() const {
  return _off_all_lights;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::is_identity
//       Access: Published
//  Description: Returns true if this is an identity attrib: it does
//               not change the set of lights in use.
////////////////////////////////////////////////////////////////////
INLINE bool LightAttrib::
is_identity() const {
  return _on_lights.empty() && _off_lights.empty() && !_off_all_lights;
}
