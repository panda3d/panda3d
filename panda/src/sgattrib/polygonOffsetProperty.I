// Filename: polygonOffsetProperty.I
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
//     Function: PolygonOffsetProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PolygonOffsetProperty::
PolygonOffsetProperty() :
  _units(0), _factor(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PolygonOffsetProperty::
PolygonOffsetProperty(int units, int factor) :
  _units(units), _factor(factor)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetProperty::set_units
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PolygonOffsetProperty::
set_units(int units) {
  _units = units;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetProperty::get_units
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PolygonOffsetProperty::
get_units() const {
  return _units;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetProperty::set_factor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PolygonOffsetProperty::
set_factor(int factor) {
  _factor = factor;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetProperty::get_factor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PolygonOffsetProperty::
get_factor() const {
  return _factor;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetProperty::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PolygonOffsetProperty::
compare_to(const PolygonOffsetProperty &other) const {
  if (_units != other._units)
  {
    return (other._units - _units);
  }

  return (other._factor - _factor);
}
