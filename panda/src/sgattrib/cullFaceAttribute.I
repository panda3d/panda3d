// Filename: cullFaceAttribute.I
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
//     Function: CullFaceAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CullFaceAttribute::
CullFaceAttribute() :
  _value(CullFaceProperty::M_cull_none)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttribute::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CullFaceAttribute::
set_mode(CullFaceProperty::Mode mode) {
  _value.set_mode(mode);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttribute::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CullFaceProperty::Mode CullFaceAttribute::
get_mode() const {
  return _value.get_mode();
}
