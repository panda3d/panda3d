// Filename: nodeAttribute.cxx
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nodeAttribute.h"

#include <indent.h>

TypeHandle NodeAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::merge
//       Access: Public, Virtual
//  Description: Attempts to merge this attribute with the other one,
//               if that makes sense to do.  Returns a new
//               NodeAttribute pointer that represents the merge, or
//               if the merge is not possible, returns the "other"
//               pointer unchanged (which is the result of the merge).
////////////////////////////////////////////////////////////////////
NodeAttribute *NodeAttribute::
merge(const NodeAttribute *other) const {
  return (NodeAttribute *)other;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeAttribute::
output(ostream &out) const {
  out << get_handle();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeAttribute::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void NodeAttribute::
issue(GraphicsStateGuardianBase *) {
}
