// Filename: onOffAttribute.cxx
// Created by:  drose (20Mar00)
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

#include "onOffAttribute.h"
#include "onOffTransition.h"

#include <indent.h>

TypeHandle OnOffAttribute::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void OnOffAttribute::
output(ostream &out) const {
  if (_is_on) {
    out << "on-" << get_handle() << ":";
    output_value(out);
  } else {
    out << "off-" << get_handle();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void OnOffAttribute::
write(ostream &out, int indent_level) const {
  if (_is_on) {
    indent(out, indent_level) << "on-" << get_handle() << ":\n";
    write_value(out, indent_level + 2);
  } else {
    indent(out, indent_level) << "off-" << get_handle() << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::internal_compare_to
//       Access: Protected, Virtual
//  Description: Returns a number < 0 if this attribute sorts before
//               the other attribute, > 0 if it sorts after, 0 if
//               they are equivalent (except for priority).
////////////////////////////////////////////////////////////////////
int OnOffAttribute::
internal_compare_to(const NodeAttribute *other) const {
  const OnOffAttribute *ot;
  DCAST_INTO_R(ot, other, false);

  if (_is_on != ot->_is_on) {
    return (_is_on < ot->_is_on) ? -1 : 1;
  }
  if (_is_on) {
    return compare_values(ot);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Assigns the value from the corresponding value of the
//               other attribute, which is assumed to be of an
//               equivalent type.
////////////////////////////////////////////////////////////////////
void OnOffAttribute::
set_value_from(const OnOffTransition *) {
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::is_value_equivalent
//       Access: Protected, Virtual
//  Description: Returns 0 if the two transitions share the same
//               value, or < 0 or > 0 otherwise.  It is given that
//               they are of equivalent types and that they are both
//               "on" transitions.
////////////////////////////////////////////////////////////////////
int OnOffAttribute::
compare_values(const OnOffAttribute *) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::output_value
//       Access: Protected, Virtual
//  Description: Outputs the value on one line.
////////////////////////////////////////////////////////////////////
void OnOffAttribute::
output_value(ostream &) const {
}

////////////////////////////////////////////////////////////////////
//     Function: OnOffAttribute::write_value
//       Access: Protected, Virtual
//  Description: Outputs the value on multiple lines.
////////////////////////////////////////////////////////////////////
void OnOffAttribute::
write_value(ostream &, int) const {
}
