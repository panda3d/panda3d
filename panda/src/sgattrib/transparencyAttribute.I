// Filename: transparencyAttribute.I
// Created by:  drose (24Mar00)
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
//     Function: TransparencyAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TransparencyAttribute::
TransparencyAttribute() :
  _value(TransparencyProperty::M_none)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyAttribute::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TransparencyProperty::Mode TransparencyAttribute::
get_mode() const {
  return _value.get_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyAttribute::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TransparencyAttribute::
set_mode(TransparencyProperty::Mode mode) {
  _value.set_mode(mode);
}
