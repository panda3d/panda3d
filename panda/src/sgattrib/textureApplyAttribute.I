// Filename: textureApplyAttribute.I
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
//     Function: TextureApplyAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextureApplyAttribute::
TextureApplyAttribute() :
  _value(TextureApplyProperty::M_modulate)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyAttribute::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TextureApplyAttribute::
set_mode(TextureApplyProperty::Mode mode) {
  _value.set_mode(mode);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyAttribute::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextureApplyProperty::Mode TextureApplyAttribute::
get_mode() const {
  return _value.get_mode();
}
