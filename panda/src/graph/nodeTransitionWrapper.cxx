// Filename: nodeTransitionWrapper.cxx
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nodeTransitionWrapper.h"
#include "nodeAttributeWrapper.h"
#include "nodeRelation.h"

#include <indent.h>

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::init_from
//       Access: Public, Static
//  Description: This is a named constructor that creates an empty
//               NodeTransitionWrapper ready to access the same type
//               of NodeTransition as the other.
////////////////////////////////////////////////////////////////////
NodeTransitionWrapper NodeTransitionWrapper::
init_from(const NodeAttributeWrapper &attrib) {
  return NodeTransitionWrapper(attrib.get_handle());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeTransitionWrapper::
output(ostream &out) const {
  if (!_entry.has_trans()) {
    out << "identity-" << _handle;
  } else {
    out << *_entry.get_trans();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionWrapper::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeTransitionWrapper::
write(ostream &out, int indent_level) const {
  if (!_entry.has_trans()) {
    indent(out, indent_level) << "identity-" << _handle << "\n";
  } else {
    _entry.get_trans()->write(out, indent_level);
  }
}
