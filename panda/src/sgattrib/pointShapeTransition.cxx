// Filename: pointShapeTransition.cxx
// Created by:  charles (11Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "pointShapeTransition.h"
#include "pointShapeAttribute.h"

#include <indent.h>

TypeHandle PointShapeTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Pointshapetransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated Pointshapetransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *PointShapeTransition::
make_copy() const {
  return new PointShapeTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Pointshapetransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated pointShapeattribute
////////////////////////////////////////////////////////////////////
NodeAttribute *PointShapeTransition::
make_attrib() const {
  return new PointShapeAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeTransition
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another PointShapeTransition.
////////////////////////////////////////////////////////////////////
void PointShapeTransition::
set_value_from(const OnTransition *other) {
  const PointShapeTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeTransition::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int PointShapeTransition::
compare_values(const OnTransition *other) const {
  const PointShapeTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void PointShapeTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void PointShapeTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
