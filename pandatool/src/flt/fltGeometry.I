// Filename: fltGeometry.I
// Created by:  drose (28Feb01)
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
  return ((_flags & F_no_color) == 0) || has_material();
}
