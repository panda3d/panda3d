/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltGeometry.I
 * @author drose
 * @date 2001-02-28
 */

/**
 * Returns true if the face has a texture applied, false otherwise.
 */
INLINE bool FltGeometry::
has_texture() const {
  return (_texture_index >= 0 && _header->has_texture(_texture_index));
}

/**
 * Returns the texture applied to this face, or NULL if no texture was
 * applied.
 */
INLINE FltTexture *FltGeometry::
get_texture() const {
  return _header->get_texture(_texture_index);
}

/**
 * Applies the indicated texture to this face, or if the texture is NULL,
 * clears it.
 */
INLINE void FltGeometry::
set_texture(FltTexture *texture) {
  if (texture == nullptr) {
    _texture_index = -1;
  } else {
    _header->add_texture(texture);
    _texture_index = texture->_pattern_index;
  }
}

/**
 * Returns true if the face has a material applied, false otherwise.
 */
INLINE bool FltGeometry::
has_material() const {
  return (_material_index >= 0 && _header->has_material(_material_index));
}

/**
 * Returns the material applied to this face, or NULL if no material was
 * applied.
 */
INLINE FltMaterial *FltGeometry::
get_material() const {
  return _header->get_material(_material_index);
}

/**
 * Applies the indicated material to this face, or if the material is NULL,
 * clears it.
 */
INLINE void FltGeometry::
set_material(FltMaterial *material) {
  if (material == nullptr) {
    _material_index = -1;
  } else {
    _header->add_material(material);
    _material_index = material->_material_index;
  }
}

/**
 * Returns true if the face has a primary color indicated, false otherwise.
 */
INLINE bool FltGeometry::
has_color() const {
  // Even if the no_color bit is not set, if the color_index is -1, the face
  // doesn't have a color (unless we've got packed color). On the other hand,
  // if we have a material than we always have color.
  return ((_flags & F_no_color) == 0 &&
          (_color_index != -1 || ((_flags & F_packed_color) != 0)))
    || has_material();
}
