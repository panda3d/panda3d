// Filename: renderModeAttribute.cxx
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "renderModeAttribute.h"
#include "renderModeTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle RenderModeAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle RenderModeAttribute::
get_handle() const {
  return RenderModeTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated RenderModeAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *RenderModeAttribute::
make_copy() const {
  return new RenderModeAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated RenderModeAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *RenderModeAttribute::
make_initial() const {
  return new RenderModeAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void RenderModeAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_render_mode(this);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               RenderModeTransition.
////////////////////////////////////////////////////////////////////
void RenderModeAttribute::
set_value_from(const OnTransition *other) {
  const RenderModeTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int RenderModeAttribute::
compare_values(const OnAttribute *other) const {
  const RenderModeAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void RenderModeAttribute::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void RenderModeAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
