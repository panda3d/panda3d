// Filename: colorMaskTransition.cxx
// Created by:  mike (08Feb99)
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

#include "colorMaskTransition.h"
#include "colorMaskAttribute.h"

#include <indent.h>

TypeHandle ColorMaskTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMaskTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *ColorMaskTransition::
make_copy() const {
  return new ColorMaskTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMaskAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorMaskTransition::
make_attrib() const {
  return new ColorMaskAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another ColorMaskTransition.
////////////////////////////////////////////////////////////////////
void ColorMaskTransition::
set_value_from(const OnTransition *other) {
  const ColorMaskTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int ColorMaskTransition::
compare_values(const OnTransition *other) const {
  const ColorMaskTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ColorMaskTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ColorMaskTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
