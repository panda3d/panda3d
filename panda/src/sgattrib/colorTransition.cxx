// Filename: colorTransition.cxx
// Created by:  drose (28Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "colorTransition.h"
#include "colorAttribute.h"

#include <indent.h>

TypeHandle ColorTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *ColorTransition::
make_copy() const {
  return new ColorTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorTransition::
make_attrib() const {
  return new ColorAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another ColorTransition.
////////////////////////////////////////////////////////////////////
void ColorTransition::
set_value_from(const OnOffTransition *other) {
  const ColorTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::compare_values
//       Access: Protected, Virtual
//  Description: Returns true if the two transitions have the same
//               value.  It is guaranteed that the other transition is
//               another ColorTransition, and that both are "on".
////////////////////////////////////////////////////////////////////
int ColorTransition::
compare_values(const OnOffTransition *other) const {
  const ColorTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ColorTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ColorTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
