// Filename: graphicsBuffer.I
// Created by:  drose (06Feb04)
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
//     Function: GraphicsBuffer::has_texture
//       Access: Published
//  Description: Returns true if the GraphicsBuffer is set to render
//               into a texture (because want_texture has been passed
//               to the GraphicsBuffer constructor), or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsBuffer::
has_texture() const {
  return !(_texture.is_null());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsBuffer::get_texture
//       Access: Published
//  Description: Returns the texture into which the GraphicsBuffer
//               renders, if any, or NULL if want_texture was passed
//               into the constructor as false.
//
//               If the texture is non-NULL, it may be applied to
//               geometry to be rendered for any other windows or
//               buffers that share the same GSG as this
//               GraphicsBuffer.  The effect is undefined for windows
//               that share a different GSG; usually in these cases
//               the texture will be invalid.
////////////////////////////////////////////////////////////////////
INLINE Texture *GraphicsBuffer::
get_texture() const {
  return _texture;
}
