// Filename: depthTestAttribute.cxx
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "depthTestAttribute.h"
#include "depthTestTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle DepthTestAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DepthTestAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle DepthTestAttribute::
get_handle() const {
  return DepthTestTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthTestAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *DepthTestAttribute::
make_copy() const {
  return new DepthTestAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthTestAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *DepthTestAttribute::
make_initial() const {
  return new DepthTestAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void DepthTestAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_depth_test(this);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               DepthTestTransition.
////////////////////////////////////////////////////////////////////
void DepthTestAttribute::
set_value_from(const OnTransition *other) {
  const DepthTestTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestAttribute::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int DepthTestAttribute::
compare_values(const OnAttribute *other) const {
  const DepthTestAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void DepthTestAttribute::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void DepthTestAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
