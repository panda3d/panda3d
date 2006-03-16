// Filename: textureContext.I
// Created by:  drose (07Oct99)
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
//     Function: TextureContext::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextureContext::
TextureContext(PreparedGraphicsObjects *pgo, Texture *tex) :
  BufferContext(&pgo->_texture_residency),
  _texture(tex)
{
#ifdef DO_PSTATS
  update_data_size_bytes(estimate_texture_memory());
#endif
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
//  Description: Returns true if the texture has been modified since the
//               last time mark_loaded() was called.
////////////////////////////////////////////////////////////////////
INLINE bool TextureContext::
was_modified() const {
  return get_modified() != _texture->get_modified();
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
  update_modified(_texture->get_modified());

  // Assume the texture is now resident.
  set_resident(true);
}
