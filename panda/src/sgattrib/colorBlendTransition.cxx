// Filename: colorBlendTransition.cxx
// Created by:  mike (28Jan99)
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

#include "colorBlendTransition.h"

#include <indent.h>

TypeHandle ColorBlendTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorBlendTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *ColorBlendTransition::
make_copy() const {
  return new ColorBlendTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorBlendTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *ColorBlendTransition::
make_initial() const {
  return new ColorBlendTransition;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void ColorBlendTransition::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_color_blend(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another ColorBlendTransition.
////////////////////////////////////////////////////////////////////
void ColorBlendTransition::
set_value_from(const OnTransition *other) {
  const ColorBlendTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int ColorBlendTransition::
compare_values(const OnTransition *other) const {
  const ColorBlendTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ColorBlendTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ColorBlendTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
