// Filename: lightTransition.cxx
// Created by:  mike (06Feb99)
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

#include "lightTransition.h"

#include <indent.h>

TypeHandle LightTransition::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: LightTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LightTransition::
LightTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LightTransition::
LightTransition(const LightTransition &copy) : MultiTransition<PT_Light, LightNameClass>(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LightTransition::
~LightTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated LightTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *LightTransition::
make_copy() const {
  return new LightTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::make_identity
//       Access: Public, Virtual
//  Description: Returns a newly allocated LightTransition in the
//               initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *LightTransition::
make_identity() const {
  return new LightTransition;
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated LightTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *LightTransition::
make_initial() const {
  return new LightTransition;
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void LightTransition::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_light(this);
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void LightTransition::
output_property(ostream &out, const PT_Light &prop) const {
  out << *prop;
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void LightTransition::
write_property(ostream &out, const PT_Light &prop,
               int indent_level) const {
  prop->write(out, indent_level);
}
