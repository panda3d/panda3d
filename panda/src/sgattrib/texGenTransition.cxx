// Filename: texGenTransition.cxx
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

#include "texGenTransition.h"

#include <indent.h>

PT(NodeTransition) TexGenTransition::_initial;
TypeHandle TexGenTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexGenTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *TexGenTransition::
make_copy() const {
  return new TexGenTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexGenTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *TexGenTransition::
make_initial() const {
  if (_initial.is_null()) {
    _initial = new TexGenTransition;
  }
  return _initial;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void TexGenTransition::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_tex_gen(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another TexGenTransition.
////////////////////////////////////////////////////////////////////
void TexGenTransition::
set_value_from(const OnTransition *other) {
  const TexGenTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int TexGenTransition::
compare_values(const OnTransition *other) const {
  const TexGenTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void TexGenTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void TexGenTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
