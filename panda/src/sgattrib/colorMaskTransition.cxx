// Filename: colorMaskTransition.cxx
// Created by:  mike (08Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "colorMaskTransition.h"
#include "colorMaskAttribute.h"

#include <indent.h>

TypeHandle ColorMaskTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMaskTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *ColorMaskTransition::
make_copy() const {
  return new ColorMaskTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMaskAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorMaskTransition::
make_attrib() const {
  return new ColorMaskAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another ColorMaskTransition.
////////////////////////////////////////////////////////////////////
void ColorMaskTransition::
set_value_from(const OnTransition *other) {
  const ColorMaskTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int ColorMaskTransition::
compare_values(const OnTransition *other) const {
  const ColorMaskTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ColorMaskTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ColorMaskTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
