// Filename: texGenAttrib.I
// Created by:  masad (21Jun04)
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
//     Function: TexGenAttrib::Constructor
//       Access: Private
//  Description: Use TexGenAttrib::make() to construct a new
//               TexGenAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TexGenAttrib::
TexGenAttrib(TexGenAttrib::Mode mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::is_off
//       Access: Published
//  Description: Returns true if the TexGenAttrib is an 'off'
//               TexGenAttrib, indicating that it should disable
//               texturing.
////////////////////////////////////////////////////////////////////
INLINE bool TexGenAttrib::
is_off() const {
  return _mode == M_nothing;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::get_texture
//       Access: Published
//  Description: If the TexGenAttrib is not an 'off' TexGenAttrib,
//               returns the texture that is associated.  Otherwise,
//               return NULL.
////////////////////////////////////////////////////////////////////
INLINE TexGenAttrib::Mode TexGenAttrib::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttrib::get_texture
//       Access: Published
//  Description: If the TexGenAttrib is not an 'off' TexGenAttrib,
//               returns the texture that is associated.  Otherwise,
//               return NULL.
////////////////////////////////////////////////////////////////////
INLINE Texture *TexGenAttrib::
get_texture() const {
  return _texture;
}
