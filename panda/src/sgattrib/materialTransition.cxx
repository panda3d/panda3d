// Filename: materialTransition.cxx
// Created by:  mike (06Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "materialTransition.h"
#include "materialAttribute.h"

#include <indent.h>

TypeHandle MaterialTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated MaterialTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *MaterialTransition::
make_copy() const {
  return new MaterialTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated MaterialAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *MaterialTransition::
make_attrib() const {
  return new MaterialAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another MaterialTransition.
////////////////////////////////////////////////////////////////////
void MaterialTransition::
set_value_from(const OnOffTransition *other) {
  const MaterialTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
  nassertv(_value != (Material *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int MaterialTransition::
compare_values(const OnOffTransition *other) const {
  const MaterialTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return (int)(_value.p() - ot->_value.p());
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void MaterialTransition::
output_value(ostream &out) const {
  nassertv(_value != (Material *)NULL);
  out << *_value;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void MaterialTransition::
write_value(ostream &out, int indent_level) const {
  nassertv(_value != (Material *)NULL);
  indent(out, indent_level) << *_value << "\n";
}
