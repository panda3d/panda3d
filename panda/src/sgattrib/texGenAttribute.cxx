// Filename: texGenAttribute.cxx
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "texGenAttribute.h"
#include "texGenTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle TexGenAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle TexGenAttribute::
get_handle() const {
  return TexGenTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexGenAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *TexGenAttribute::
make_copy() const {
  return new TexGenAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexGenAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *TexGenAttribute::
make_initial() const {
  return new TexGenAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void TexGenAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_tex_gen(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               TexGenTransition.
////////////////////////////////////////////////////////////////////
void TexGenAttribute::
set_value_from(const OnTransition *other) {
  const TexGenTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int TexGenAttribute::
compare_values(const OnAttribute *other) const {
  const TexGenAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void TexGenAttribute::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void TexGenAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
