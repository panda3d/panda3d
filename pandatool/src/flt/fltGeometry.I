// Filename: fltGeometry.I
// Created by:  drose (28Feb01)
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
//     Function: FltGeometry::has_texture
//       Access: Public
//  Description: Returns true if the face has a texture applied, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FltGeometry::
has_texture() const {
  return (_texture_index >= 0 && _header->has_texture(_texture_index));
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::get_texture
//       Access: Public
//  Description: Returns the texture applied to this face, or NULL if
//               no texture was applied.
////////////////////////////////////////////////////////////////////
INLINE FltTexture *FltGeometry::
get_texture() const {
  return _header->get_texture(_texture_index);
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::has_material
//       Access: Public
//  Description: Returns true if the face has a material applied, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FltGeometry::
has_material() const {
  return (_material_index >= 0 && _header->has_material(_material_index));
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::get_material
//       Access: Public
//  Description: Returns the material applied to this face, or NULL if
//               no material was applied.
////////////////////////////////////////////////////////////////////
INLINE FltMaterial *FltGeometry::
get_material() const {
  return _header->get_material(_material_index);
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::has_color
//       Access: Public
//  Description: Returns true if the face has a primary color
//               indicated, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FltGeometry::
has_color() const {
  // Even if the no_color bit is not set, if the color_index is -1,
  // the face doesn't have a color (unless we've got packed color).
  // On the other hand, if we have a material than we always have
  // color.
  return ((_flags & F_no_color) == 0 &&
          (_color_index != -1 || ((_flags & F_packed_color) != 0)))
    || has_material();
}
