// Filename: colorMaskAttribute.cxx
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

#include "colorMaskAttribute.h"
#include "colorMaskTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle ColorMaskAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle ColorMaskAttribute::
get_handle() const {
  return ColorMaskTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMaskAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorMaskAttribute::
make_copy() const {
  return new ColorMaskAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMaskAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorMaskAttribute::
make_initial() const {
  return new ColorMaskAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void ColorMaskAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_color_mask(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               ColorMaskTransition.
////////////////////////////////////////////////////////////////////
void ColorMaskAttribute::
set_value_from(const OnTransition *other) {
  const ColorMaskTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int ColorMaskAttribute::
compare_values(const OnAttribute *other) const {
  const ColorMaskAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ColorMaskAttribute::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ColorMaskAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
