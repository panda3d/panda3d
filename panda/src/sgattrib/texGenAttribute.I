// Filename: texGenAttribute.I
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
//     Function: TexGenAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TexGenAttribute::
TexGenAttribute() {
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::set_none
//       Access: Public
//  Description: Sets the attribute to disable texture coordinate
//               generation.
////////////////////////////////////////////////////////////////////
INLINE void TexGenAttribute::
set_none() {
  _value = TexGenProperty();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::set_texture_projector
//       Access: Public
//  Description: Sets the attribute to project textures through a
//               perpective frustum.
////////////////////////////////////////////////////////////////////
INLINE void TexGenAttribute::
set_texture_projector() {
  _value = TexGenProperty::texture_projector();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::set_sphere_map
//       Access: Public
//  Description: Sets the attribute to project textures as sphere
//               maps.
////////////////////////////////////////////////////////////////////
INLINE void TexGenAttribute::
set_sphere_map() {
  _value = TexGenProperty::sphere_map();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TexGenProperty::Mode TexGenAttribute::
get_mode() const {
  return _value.get_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::get_plane
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LMatrix4f TexGenAttribute::
get_plane() const {
  return _value.get_plane();
}
