// Filename: depthTestProperty.I
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
//     Function: DepthTestProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DepthTestProperty::
DepthTestProperty(DepthTestProperty::Mode mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestProperty::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DepthTestProperty::
set_mode(DepthTestProperty::Mode mode) {
  _mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestProperty::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DepthTestProperty::Mode DepthTestProperty::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestProperty::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int DepthTestProperty::
compare_to(const DepthTestProperty &other) const {
  return (int)_mode - (int)other._mode;
}
