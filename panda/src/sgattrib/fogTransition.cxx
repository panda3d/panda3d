// Filename: fogTransition.cxx
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

#include "fogTransition.h"
#include "fogAttribute.h"

#include <indent.h>

TypeHandle FogTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FogTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated FogTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *FogTransition::
make_copy() const {
  return new FogTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: FogTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated FogAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *FogTransition::
make_attrib() const {
  return new FogAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: FogTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another FogTransition.
////////////////////////////////////////////////////////////////////
void FogTransition::
set_value_from(const OnOffTransition *other) {
  const FogTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
  nassertv(_value != (Fog *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: FogTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int FogTransition::
compare_values(const OnOffTransition *other) const {
  const FogTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return (int)(_value.p() - ot->_value.p());
}

////////////////////////////////////////////////////////////////////
//     Function: FogTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void FogTransition::
output_value(ostream &out) const {
  nassertv(_value != (Fog *)NULL);
  out << *_value;
}

////////////////////////////////////////////////////////////////////
//     Function: FogTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void FogTransition::
write_value(ostream &out, int indent_level) const {
  nassertv(_value != (Fog *)NULL);
  indent(out, indent_level) << *_value << "\n";
}
