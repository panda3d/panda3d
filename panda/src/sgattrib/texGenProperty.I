// Filename: texGenProperty.I
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
//     Function: TexGenProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TexGenProperty::
TexGenProperty() :
  _mode(M_none)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenProperty::texture_projector
//       Access: Public
//  Description: This named constructor creates a TexGenProperty
//               suitable for projecting textures through a
//               perspective frustum.
////////////////////////////////////////////////////////////////////
INLINE TexGenProperty TexGenProperty::
texture_projector() {
  TexGenProperty p;
  p._mode = M_texture_projector;
  p._plane = LMatrix4f::ident_mat();
  return p;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenProperty::sphere_map
//       Access: Public
//  Description: This named constructor creates a TexGenProperty
//               suitable for applying textures as a sphere map.
////////////////////////////////////////////////////////////////////
INLINE TexGenProperty TexGenProperty::
sphere_map() {
  TexGenProperty p;
  p._mode = M_sphere_map;
  return p;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenProperty::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TexGenProperty::Mode TexGenProperty::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenProperty::get_plane
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &TexGenProperty::
get_plane() const {
  return _plane;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenProperty::Equality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int TexGenProperty::
compare_to(const TexGenProperty &other) const {
  return (int)_mode - (int)other._mode;
  // Should we compare planes also?
}
