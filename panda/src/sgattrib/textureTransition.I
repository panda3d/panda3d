// Filename: textureTransition.I
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
//     Function: TextureTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextureTransition::
TextureTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextureTransition::
TextureTransition(Texture *texture) :
  OnOffTransition(TD_on),
  _value(texture)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::off
//       Access: Public
//  Description: This named constructor returns a TextureTransition that
//               turns off texturing, thus rendering geometry
//               untextured.
////////////////////////////////////////////////////////////////////
INLINE TextureTransition TextureTransition::
off() {
  TextureTransition result;
  result.set_off();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::set_on
//       Access: Public
//  Description: Changes the TextureTransition to render the particular
//               texture.
////////////////////////////////////////////////////////////////////
INLINE void TextureTransition::
set_on(Texture *texture) {
  nassertv(texture != (Texture *)NULL);
  _value = texture;
  OnOffTransition::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::get_texture
//       Access: Public
//  Description: Returns the texture that the TextureTransition
//               represents.  It is only valid to call this if is_on()
//               has returned true.
////////////////////////////////////////////////////////////////////
INLINE PT(Texture) TextureTransition::
get_texture() const {
  nassertr(is_on(), NULL);
  return _value;
}
