// Filename: polygonOffsetTransition.cxx
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

#include "polygonOffsetTransition.h"

#include <indent.h>

PT(NodeTransition) PolygonOffsetTransition::_initial;
TypeHandle PolygonOffsetTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated PolygonOffsetTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *PolygonOffsetTransition::
make_copy() const {
  return new PolygonOffsetTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated PolygonOffsetTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *PolygonOffsetTransition::
make_initial() const {
  if (_initial.is_null()) {
    _initial = new PolygonOffsetTransition;
  }
  return _initial;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void PolygonOffsetTransition::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_polygon_offset(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another PolygonOffsetTransition.
////////////////////////////////////////////////////////////////////
void PolygonOffsetTransition::
set_value_from(const OnTransition *other) {
  const PolygonOffsetTransition *ot;
  DCAST_INTO_V(ot, other);
  _state = ot->_state;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int PolygonOffsetTransition::
compare_values(const OnTransition *other) const {
  const PolygonOffsetTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _state.compare_to(ot->_state);
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void PolygonOffsetTransition::
output_value(ostream &out) const {
  out << _state;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void PolygonOffsetTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _state << "\n";
}







