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
  _operation = O_set;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_operation
//       Access: Published
//  Description: Returns the basic operation type of the LightAttrib.
//               If this is O_set, the lights listed here completely
//               replace any lights that were already on.  If this is
//               O_add, the lights here are added to the set of of
//               lights that were already on, and if O_remove, the
//               lights here are removed from the set of lights that
//               were on.
////////////////////////////////////////////////////////////////////
INLINE LightAttrib::Operation LightAttrib::
get_operation() const {
  return _operation;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_num_lights
//       Access: Published
//  Description: Returns the number of lights listed in the attribute.
////////////////////////////////////////////////////////////////////
INLINE int LightAttrib::
get_num_lights() const {
  return _lights.size();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::get_light
//       Access: Published
//  Description: Returns the nth lights listed in the attribute.
////////////////////////////////////////////////////////////////////
INLINE Light *LightAttrib::
get_light(int n) const {
  nassertr(n >= 0 && n < (int)_lights.size(), (Light *)NULL);
  return _lights[n];
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::add_light
//       Access: Published
//  Description: Returns a new LightAttrib, just like this one, but
//               with the indicated light added to the list of lights.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) LightAttrib::
add_light(Light *light) const {
  if (_operation == O_remove) {  
    return compose(make(O_remove, light));
  } else {
    return compose(make(O_add, light));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::remove_light
//       Access: Published
//  Description: Returns a new LightAttrib, just like this one, but
//               with the indicated light removed from the list of
//               lights.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) LightAttrib::
remove_light(Light *light) const {
  if (_operation == O_remove) {  
    return compose(make(O_add, light));
  } else {
    return compose(make(O_remove, light));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::is_identity
//       Access: Published
//  Description: Returns true if this is an identity attrib: it does
//               not change the set of lights in use.
////////////////////////////////////////////////////////////////////
INLINE bool LightAttrib::
is_identity() const {
  return _operation != O_set && _lights.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::is_all_off
//       Access: Published
//  Description: Returns true if this attrib turns off all lights and
//               turns none on.
////////////////////////////////////////////////////////////////////
INLINE bool LightAttrib::
is_all_off() const {
  return _operation == O_set && _lights.empty();
}
