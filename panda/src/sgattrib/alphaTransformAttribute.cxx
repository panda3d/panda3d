// Filename: alphaTransformAttribute.cxx
// Created by:  jason (01Aug00)
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

#include "alphaTransformAttribute.h"
#include "alphaTransformTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle AlphaTransformAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle AlphaTransformAttribute::
get_handle() const {
  return AlphaTransformTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated AlphaTransformAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *AlphaTransformAttribute::
make_copy() const {
  return new AlphaTransformAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated AlphaTransformAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *AlphaTransformAttribute::
make_initial() const {
  return new AlphaTransformAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void AlphaTransformAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_alpha_transform(this);
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               AlphaTransformTransition.
////////////////////////////////////////////////////////////////////
void AlphaTransformAttribute::
set_value_from(const OnTransition *other) {
  const AlphaTransformTransition *ot;
  DCAST_INTO_V(ot, other);
  _state = ot->_state;
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int AlphaTransformAttribute::
compare_values(const OnAttribute *other) const {
  const AlphaTransformAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _state.compare_to(ot->_state);
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void AlphaTransformAttribute::
output_value(ostream &out) const {
  out << _state;
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void AlphaTransformAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _state << "\n";
}


