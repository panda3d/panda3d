// Filename: textureAttribute.I
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
//     Function: TextureAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextureAttribute::
TextureAttribute() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::set_on
//       Access: Public
//  Description: Changes the TextureAttribute to render the particular
//               texture.
////////////////////////////////////////////////////////////////////
INLINE void TextureAttribute::
set_on(Texture *texture) {
  nassertv(texture != (Texture *)NULL);
  _value = texture;
  OnOffAttribute::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::get_texture
//       Access: Public
//  Description: Returns the texture that the TextureAttribute
//               represents.  It is only valid to call this if is_on()
//               has returned true.
////////////////////////////////////////////////////////////////////
INLINE PT(Texture) TextureAttribute::
get_texture() const {
  nassertr(is_on(), NULL);
  return _value;
}
