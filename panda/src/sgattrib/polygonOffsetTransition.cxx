// Filename: polygonOffsetTransition.cxx
// Created by:  jason (12Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "polygonOffsetTransition.h"
#include "polygonOffsetAttribute.h"

#include <indent.h>

TypeHandle PolygonOffsetTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated PolygonOffsetTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *PolygonOffsetTransition::
make_copy() const {
  return new PolygonOffsetTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated PolygonOffsetAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *PolygonOffsetTransition::
make_attrib() const {
  return new PolygonOffsetAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another PolygonOffsetTransition.
////////////////////////////////////////////////////////////////////
void PolygonOffsetTransition::
set_value_from(const OnTransition *other) {
  const PolygonOffsetTransition *ot;
  DCAST_INTO_V(ot, other);
  _state = ot->_state;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int PolygonOffsetTransition::
compare_values(const OnTransition *other) const {
  const PolygonOffsetTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _state.compare_to(ot->_state);
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void PolygonOffsetTransition::
output_value(ostream &out) const {
  out << _state;
}

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void PolygonOffsetTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _state << "\n";
}







