// Filename: pointShapeTransition.cxx
// Created by:  charles (11Jul00)
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

#include "pointShapeTransition.h"
#include "pointShapeAttribute.h"

#include <indent.h>

TypeHandle PointShapeTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Pointshapetransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated Pointshapetransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *PointShapeTransition::
make_copy() const {
  return new PointShapeTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Pointshapetransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated pointShapeattribute
////////////////////////////////////////////////////////////////////
NodeAttribute *PointShapeTransition::
make_attrib() const {
  return new PointShapeAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeTransition
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another PointShapeTransition.
////////////////////////////////////////////////////////////////////
void PointShapeTransition::
set_value_from(const OnTransition *other) {
  const PointShapeTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int PointShapeTransition::
compare_values(const OnTransition *other) const {
  const PointShapeTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void PointShapeTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void PointShapeTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
