// Filename: transparencyAttribute.cxx
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "transparencyAttribute.h"
#include "transparencyTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle TransparencyAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TransparencyAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle TransparencyAttribute::
get_handle() const {
  return TransparencyTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TransparencyAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *TransparencyAttribute::
make_copy() const {
  return new TransparencyAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated TransparencyAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *TransparencyAttribute::
make_initial() const {
  return new TransparencyAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void TransparencyAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_transparency(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               TransparencyTransition.
////////////////////////////////////////////////////////////////////
void TransparencyAttribute::
set_value_from(const OnTransition *other) {
  const TransparencyTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyAttribute::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int TransparencyAttribute::
compare_values(const OnAttribute *other) const {
  const TransparencyAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void TransparencyAttribute::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void TransparencyAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
