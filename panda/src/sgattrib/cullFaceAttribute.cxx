// Filename: cullFaceAttribute.cxx
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "cullFaceAttribute.h"
#include "cullFaceTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle CullFaceAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle CullFaceAttribute::
get_handle() const {
  return CullFaceTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated CullFaceAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *CullFaceAttribute::
make_copy() const {
  return new CullFaceAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated CullFaceAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *CullFaceAttribute::
make_initial() const {
  return new CullFaceAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void CullFaceAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_cull_face(this);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               CullFaceTransition.
////////////////////////////////////////////////////////////////////
void CullFaceAttribute::
set_value_from(const OnTransition *other) {
  const CullFaceTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttribute::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int CullFaceAttribute::
compare_values(const OnAttribute *other) const {
  const CullFaceAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void CullFaceAttribute::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void CullFaceAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}
