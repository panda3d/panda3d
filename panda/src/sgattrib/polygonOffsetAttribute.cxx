// Filename: polygonOffsetAttribute.cxx
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

#include "polygonOffsetAttribute.h"
#include "polygonOffsetTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle PolygonOffsetAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle PolygonOffsetAttribute::
get_handle() const {
  return PolygonOffsetTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated PolygonOffsetAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *PolygonOffsetAttribute::
make_copy() const {
  return new PolygonOffsetAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated PolygonOffsetAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *PolygonOffsetAttribute::
make_initial() const {
  return new PolygonOffsetAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void PolygonOffsetAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_polygon_offset(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               PolygonOffsetTransition.
////////////////////////////////////////////////////////////////////
void PolygonOffsetAttribute::
set_value_from(const OnTransition *other) {
  const PolygonOffsetTransition *ot;
  DCAST_INTO_V(ot, other);
  _state = ot->_state;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int PolygonOffsetAttribute::
compare_values(const OnAttribute *other) const {
  const PolygonOffsetAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _state.compare_to(ot->_state);
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void PolygonOffsetAttribute::
output_value(ostream &out) const {
  out << _state;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void PolygonOffsetAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _state << "\n";
}


