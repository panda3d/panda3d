// Filename: textureContext.I
// Created by:  drose (07Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TextureContext::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextureContext::
TextureContext(PreparedGraphicsObjects *pgo, Texture *tex) :
  BufferContext(&pgo->_texture_residency),
  _texture(tex)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextureContext::get_texture
//       Access: Public
//  Description: Returns the pointer to the associated Texture
//               object.
////////////////////////////////////////////////////////////////////
INLINE Texture *TextureContext::
get_texture() const {
  return _texture;
}


////////////////////////////////////////////////////////////////////
//     Function: TextureContext::was_modified
//       Access: Public
//  Description: Returns true if the texture properties or image have
//               been modified since the last time mark_loaded() was
//               called.
////////////////////////////////////////////////////////////////////
INLINE bool TextureContext::
was_modified() const {
  return was_properties_modified() || was_image_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureContext::was_properties_modified
//       Access: Public
//  Description: Returns true if the texture properties (unrelated to
//               the image) have been modified since the last time
//               mark_loaded() was called.
////////////////////////////////////////////////////////////////////
INLINE bool TextureContext::
was_properties_modified() const {
  return _properties_modified != _texture->get_properties_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureContext::was_image_modified
//       Access: Public
//  Description: Returns true if the texture image has been modified
//               since the last time mark_loaded() was called.
////////////////////////////////////////////////////////////////////
INLINE bool TextureContext::
was_image_modified() const {
  return _image_modified != _texture->get_image_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureContext::mark_loaded
//       Access: Public
//  Description: Should be called after the TextureContext has been
//               loaded into graphics memory, this updates the
//               internal flags for changed_size() and modified().
////////////////////////////////////////////////////////////////////
INLINE void TextureContext::
mark_loaded() {
  //  _data_size_bytes = _data->get_texture_size_bytes();
  _properties_modified = _texture->get_properties_modified();
  _image_modified = _texture->get_image_modified();
  update_modified(max(_properties_modified, _image_modified));

  // Assume the texture is now resident.
  set_resident(true);
}
