// Filename: renderModeTransition.cxx
// Created by:  drose (08Feb99)
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

#include "renderModeTransition.h"

#include <indent.h>

PT(NodeTransition) RenderModeTransition::_initial;
TypeHandle RenderModeTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated RenderModeTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *RenderModeTransition::
make_copy() const {
  return new RenderModeTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated RenderModeTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *RenderModeTransition::
make_initial() const {
  if (_initial.is_null()) {
    _initial = new RenderModeTransition;
  }
  return _initial;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void RenderModeTransition::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_render_mode(this);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another RenderModeTransition.
////////////////////////////////////////////////////////////////////
void RenderModeTransition::
set_value_from(const OnTransition *other) {
  const RenderModeTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int RenderModeTransition::
compare_values(const OnTransition *other) const {
  const RenderModeTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void RenderModeTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void RenderModeTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
