// Filename: pointShapeProperty.I
// Created by:  charles (10Jul00)
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
// Function : PointShapeProperty
//   Access : public
////////////////////////////////////////////////////////////////////

INLINE PointShapeProperty::
PointShapeProperty(PointShapeProperty::Mode mode) :
  _mode(mode) {
}

////////////////////////////////////////////////////////////////////
// Function : set_mode
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void PointShapeProperty::
set_mode(PointShapeProperty::Mode mode) {
  _mode = mode;
}

////////////////////////////////////////////////////////////////////
// Function : get_mode
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE PointShapeProperty::Mode PointShapeProperty::
get_mode(void) const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
// Function : compare_to
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE int PointShapeProperty::
compare_to(const PointShapeProperty& other) const {
  if (_mode != other._mode) {
    return (int) _mode - (int) other._mode;
  }

  return 0;
}
