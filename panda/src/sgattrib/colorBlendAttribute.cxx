// Filename: colorBlendAttribute.cxx
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "colorBlendAttribute.h"
#include "colorBlendTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle ColorBlendAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle ColorBlendAttribute::
get_handle() const {
  return ColorBlendTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorBlendAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorBlendAttribute::
make_copy() const {
  return new ColorBlendAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorBlendAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorBlendAttribute::
make_initial() const {
  return new ColorBlendAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void ColorBlendAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_color_blend(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               ColorBlendTransition.
////////////////////////////////////////////////////////////////////
void ColorBlendAttribute::
set_value_from(const OnTransition *other) {
  const ColorBlendTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttribute::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int ColorBlendAttribute::
compare_values(const OnAttribute *other) const {
  const ColorBlendAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ColorBlendAttribute::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ColorBlendAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
