// Filename: stencilTransition.cxx
// Created by:  mike (28Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "stencilTransition.h"
#include "stencilAttribute.h"

#include <indent.h>

TypeHandle StencilTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated StencilTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *StencilTransition::
make_copy() const {
  return new StencilTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated StencilAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *StencilTransition::
make_attrib() const {
  return new StencilAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another StencilTransition.
////////////////////////////////////////////////////////////////////
void StencilTransition::
set_value_from(const OnTransition *other) {
  const StencilTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int StencilTransition::
compare_values(const OnTransition *other) const {
  const StencilTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void StencilTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void StencilTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
