// Filename: textureApplyAttribute.cxx
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "textureApplyAttribute.h"
#include "textureApplyTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle TextureApplyAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle TextureApplyAttribute::
get_handle() const {
  return TextureApplyTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TextureApplyAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *TextureApplyAttribute::
make_copy() const {
  return new TextureApplyAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated TextureApplyAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *TextureApplyAttribute::
make_initial() const {
  return new TextureApplyAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void TextureApplyAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_texture_apply(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               TextureApplyTransition.
////////////////////////////////////////////////////////////////////
void TextureApplyAttribute::
set_value_from(const OnTransition *other) {
  const TextureApplyTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyAttribute::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int TextureApplyAttribute::
compare_values(const OnAttribute *other) const {
  const TextureApplyAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void TextureApplyAttribute::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void TextureApplyAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
