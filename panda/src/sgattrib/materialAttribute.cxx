// Filename: materialAttribute.cxx
// Created by:  drose (22Mar00)
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

#include "materialAttribute.h"
#include "materialTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle MaterialAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle MaterialAttribute::
get_handle() const {
  return MaterialTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated MaterialAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *MaterialAttribute::
make_copy() const {
  return new MaterialAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated MaterialAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *MaterialAttribute::
make_initial() const {
  return new MaterialAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void MaterialAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_material(this);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               MaterialTransition.
////////////////////////////////////////////////////////////////////
void MaterialAttribute::
set_value_from(const OnOffTransition *other) {
  const MaterialTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
  nassertv(_value != (const Material *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int MaterialAttribute::
compare_values(const OnOffAttribute *other) const {
  const MaterialAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return (int)(_value.p() - ot->_value.p());
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void MaterialAttribute::
output_value(ostream &out) const {
  nassertv(_value != (const Material *)NULL);
  out << *_value;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void MaterialAttribute::
write_value(ostream &out, int indent_level) const {
  nassertv(_value != (const Material *)NULL);
  indent(out, indent_level) << *_value << "\n";
}
