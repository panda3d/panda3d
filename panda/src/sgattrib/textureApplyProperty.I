// Filename: textureApplyProperty.I
// Created by:  drose (23Mar00)
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
//     Function: TextureApplyProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextureApplyProperty::
TextureApplyProperty(TextureApplyProperty::Mode mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::Constructor
//       Access: Public
//  Description: This should only be called in the Protected
//               constructor of TextureApplyTransition.  It is made
//               public, only for that to access, and is necessary
//               for creating an object in make_TextureApplyTransition
//               before any information has been read
////////////////////////////////////////////////////////////////////
INLINE TextureApplyProperty::
TextureApplyProperty(void)
{
}


////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextureApplyProperty::
set_mode(TextureApplyProperty::Mode mode) {
  _mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextureApplyProperty::Mode TextureApplyProperty::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int TextureApplyProperty::
compare_to(const TextureApplyProperty &other) const {
  return (int)_mode - (int)other._mode;
}
