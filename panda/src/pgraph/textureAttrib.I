// Filename: textureAttrib.I
// Created by:  drose (21Feb02)
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
//     Function: TextureAttrib::Constructor
//       Access: Private
//  Description: Use TextureAttrib::make() to construct a new
//               TextureAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TextureAttrib::
TextureAttrib() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::is_off
//       Access: Published
//  Description: Returns true if the TextureAttrib is an 'off'
//               TextureAttrib, indicating that it should disable
//               texturing.
////////////////////////////////////////////////////////////////////
INLINE bool TextureAttrib::
is_off() const {
  return _texture == (Texture *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::get_texture
//       Access: Published
//  Description: If the TextureAttrib is not an 'off' TextureAttrib,
//               returns the texture that is associated.  Otherwise,
//               return NULL.
////////////////////////////////////////////////////////////////////
INLINE Texture *TextureAttrib::
get_texture() const {
  return _texture;
}
