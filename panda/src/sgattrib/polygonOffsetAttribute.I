// Filename: polygonOffsetAttribute.I
// Created by:  jason (12Jul00)
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
//     Function: PolygonOffsetAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PolygonOffsetAttribute::
PolygonOffsetAttribute()
{
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::set_units
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PolygonOffsetAttribute::
set_units(int units) {
  _state.set_units(units);
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::get_units
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PolygonOffsetAttribute::
get_units() const {
  return _state.get_units();
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::set_factor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PolygonOffsetAttribute::
set_factor(int factor) {
  _state.set_factor(factor);
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::get_factor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PolygonOffsetAttribute::
get_factor() const {
  return _state.get_factor();
}
