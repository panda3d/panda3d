// Filename: fogAttribute.cxx
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "fogAttribute.h"
#include "fogTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle FogAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FogAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle FogAttribute::
get_handle() const {
  return FogTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated FogAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *FogAttribute::
make_copy() const {
  return new FogAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated FogAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *FogAttribute::
make_initial() const {
  return new FogAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void FogAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_fog(this);
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               FogTransition.
////////////////////////////////////////////////////////////////////
void FogAttribute::
set_value_from(const OnOffTransition *other) {
  const FogTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
  nassertv(_value != (Fog *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttribute::compare_values
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int FogAttribute::
compare_values(const OnOffAttribute *other) const {
  const FogAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return (int)(_value.p() - ot->_value.p());
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void FogAttribute::
output_value(ostream &out) const {
  nassertv(_value != (Fog *)NULL);
  out << *_value;
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void FogAttribute::
write_value(ostream &out, int indent_level) const {
  nassertv(_value != (Fog *)NULL);
  indent(out, indent_level) << *_value << "\n";
}
