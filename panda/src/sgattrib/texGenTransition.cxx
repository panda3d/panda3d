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
#include "texGenAttribute.h"

#include <indent.h>

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
//     Function: TexGenTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexGenAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *TexGenTransition::
make_attrib() const {
  return new TexGenAttribute;
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
