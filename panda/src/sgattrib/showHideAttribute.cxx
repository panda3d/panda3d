// Filename: showHideAttribute.cxx
// Created by:  drose (26Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "showHideAttribute.h"
#include "showHideTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle ShowHideAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ShowHideAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle ShowHideAttribute::
get_handle() const {
  return ShowHideTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ShowHideAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *ShowHideAttribute::
make_copy() const {
  return new ShowHideAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated ShowHideAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *ShowHideAttribute::
make_initial() const {
  return new ShowHideAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ShowHideAttribute::
output_property(ostream &out, const PT(Camera) &prop) const {
  out << *prop;
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ShowHideAttribute::
write_property(ostream &out, const PT(Camera) &prop,
               int indent_level) const {
  indent(out, indent_level) << *prop << "\n";
}
