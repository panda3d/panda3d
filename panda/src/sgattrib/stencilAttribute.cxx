// Filename: stencilAttribute.cxx
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "stencilAttribute.h"
#include "stencilTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle StencilAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle StencilAttribute::
get_handle() const {
  return StencilTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated StencilAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *StencilAttribute::
make_copy() const {
  return new StencilAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated StencilAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *StencilAttribute::
make_initial() const {
  return new StencilAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void StencilAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_stencil(this);
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               StencilTransition.
////////////////////////////////////////////////////////////////////
void StencilAttribute::
set_value_from(const OnTransition *other) {
  const StencilTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int StencilAttribute::
compare_values(const OnAttribute *other) const {
  const StencilAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void StencilAttribute::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void StencilAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
