// Filename: pointShapeAttribute.cxx
// Created by:  charles (11Jul00)
// (absurd abuse of cut n' paste from renderModeAttribute.cxx)
////////////////////////////////////////////////////////////////////

#include "pointShapeAttribute.h"
#include "pointShapeTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle PointShapeAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PointShapeAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle PointShapeAttribute::
get_handle() const {
  return PointShapeTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated PointShapeAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *PointShapeAttribute::
make_copy() const {
  return new PointShapeAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated PointShapeAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *PointShapeAttribute::
make_initial() const {
  return new PointShapeAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void PointShapeAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_point_shape(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               PointShapeTransition.
////////////////////////////////////////////////////////////////////
void PointShapeAttribute::
set_value_from(const OnTransition *other) {
  const PointShapeTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeAttribute::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int PointShapeAttribute::
compare_values(const OnAttribute *other) const {
  const PointShapeAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void PointShapeAttribute::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: PointShapeAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void PointShapeAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
