// Filename: lightTransition.cxx
// Created by:  mike (06Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "lightTransition.h"
#include "lightAttribute.h"

#include <indent.h>

TypeHandle LightTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated LightTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *LightTransition::
make_copy() const {
  return new LightTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated LightAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *LightTransition::
make_attrib() const {
  return new LightAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::make_identity
//       Access: Public, Virtual
//  Description: Returns a newly allocated LightTransition in the
//               initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *LightTransition::
make_identity() const {
  return new LightTransition;
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void LightTransition::
output_property(ostream &out, const PT_Light &prop) const {
  out << *prop;
}

////////////////////////////////////////////////////////////////////
//     Function: LightTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void LightTransition::
write_property(ostream &out, const PT_Light &prop,
	       int indent_level) const {
  prop->write(out, indent_level);
}
