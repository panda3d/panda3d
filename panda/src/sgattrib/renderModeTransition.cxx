// Filename: renderModeTransition.cxx
// Created by:  drose (08Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "renderModeTransition.h"
#include "renderModeAttribute.h"

#include <indent.h>

TypeHandle RenderModeTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated RenderModeTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *RenderModeTransition::
make_copy() const {
  return new RenderModeTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated RenderModeAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *RenderModeTransition::
make_attrib() const {
  return new RenderModeAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another RenderModeTransition.
////////////////////////////////////////////////////////////////////
void RenderModeTransition::
set_value_from(const OnTransition *other) {
  const RenderModeTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int RenderModeTransition::
compare_values(const OnTransition *other) const {
  const RenderModeTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void RenderModeTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void RenderModeTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
