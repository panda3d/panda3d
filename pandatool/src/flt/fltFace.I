// Filename: fltFace.I
// Created by:  drose (30Aug00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: FltFace::has_texture
//       Access: Public
//  Description: Returns true if the face has a texture applied, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FltFace::
has_texture() const {
  return (_texture_index >= 0 && _header->has_texture(_texture_index));
}

////////////////////////////////////////////////////////////////////
//     Function: FltFace::get_texture
//       Access: Public
//  Description: Returns the texture applied to this face, or NULL if
//               no texture was applied.
////////////////////////////////////////////////////////////////////
INLINE FltTexture *FltFace::
get_texture() const {
  return _header->get_texture(_texture_index);
}

////////////////////////////////////////////////////////////////////
//     Function: FltFace::has_material
//       Access: Public
//  Description: Returns true if the face has a material applied, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FltFace::
has_material() const {
  return (_material_index >= 0 && _header->has_material(_material_index));
}

////////////////////////////////////////////////////////////////////
//     Function: FltFace::get_material
//       Access: Public
//  Description: Returns the material applied to this face, or NULL if
//               no material was applied.
////////////////////////////////////////////////////////////////////
INLINE FltMaterial *FltFace::
get_material() const {
  return _header->get_material(_material_index);
}

////////////////////////////////////////////////////////////////////
//     Function: FltFace::has_color
//       Access: Public
//  Description: Returns true if the face has a primary color
//               indicated, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FltFace::
has_color() const {
  return ((_flags & F_no_color) == 0) || has_material();
}
