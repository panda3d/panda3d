// Filename: nodeAttribute.cxx
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nodeAttribute.h"

#include <indent.h>

TypeHandle NodeAttribute::_type_handle;

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
