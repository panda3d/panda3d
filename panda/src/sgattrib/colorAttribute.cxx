// Filename: colorAttribute.cxx
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "colorAttribute.h"
#include "colorTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle ColorAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle ColorAttribute::
get_handle() const {
  return ColorTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorAttribute::
make_copy() const {
  return new ColorAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorAttribute::
make_initial() const {
  return new ColorAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void ColorAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_color(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               ColorTransition.
////////////////////////////////////////////////////////////////////
void ColorAttribute::
set_value_from(const OnOffTransition *other) {
  const ColorTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::compare_values
//       Access: Protected, Virtual
//  Description: Returns true if the two attributes have the same
//               value.  It is guaranteed that the other attribute is
//               another ColorAttribute, and that both are "on".
////////////////////////////////////////////////////////////////////
int ColorAttribute::
compare_values(const OnOffAttribute *other) const {
  const ColorAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ColorAttribute::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ColorAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
