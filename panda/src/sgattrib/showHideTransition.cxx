// Filename: showHideTransition.cxx
// Created by:  drose (26Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "showHideTransition.h"
#include "showHideAttribute.h"

#include <indent.h>

TypeHandle ShowHideTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ShowHideTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ShowHideTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *ShowHideTransition::
make_copy() const {
  return new ShowHideTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated ShowHideAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *ShowHideTransition::
make_attrib() const {
  return new ShowHideAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideTransition::make_identity
//       Access: Public, Virtual
//  Description: Returns a newly allocated ShowHideTransition in the
//               initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *ShowHideTransition::
make_identity() const {
  return new ShowHideTransition;
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ShowHideTransition::
output_property(ostream &out, const PT(Camera) &prop) const {
  out << *prop;
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ShowHideTransition::
write_property(ostream &out, const PT(Camera) &prop,
               int indent_level) const {
  indent(out, indent_level) << *prop << "\n";
}
