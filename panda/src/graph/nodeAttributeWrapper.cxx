// Filename: nodeAttributeWrapper.cxx
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nodeAttributeWrapper.h"
#include "nodeTransitionWrapper.h"

#include <indent.h>

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::init_from
//       Access: Public, Static
//  Description: This is a named constructor that creates an empty
//               NodeAttributeWrapper ready to access the same type
//               of NodeAttribute as the other.
////////////////////////////////////////////////////////////////////
NodeAttributeWrapper NodeAttributeWrapper::
init_from(const NodeTransitionWrapper &trans) {
  return NodeAttributeWrapper(trans.get_handle());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::apply_in_place
//       Access: Public
//  Description: Modifies the attribute by applying the transition.
////////////////////////////////////////////////////////////////////
void NodeAttributeWrapper::
apply_in_place(const NodeTransitionWrapper &trans) {
  nassertv(_handle == trans.get_handle());
  _attrib = NodeTransitionCacheEntry::apply(_attrib, trans._entry);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeAttributeWrapper::
output(ostream &out) const {
  if (_attrib == (NodeAttribute *)NULL) {
    out << "no " << _handle;
  } else {
    out << *_attrib;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeAttributeWrapper::
write(ostream &out, int indent_level) const {
  if (_attrib == (NodeAttribute *)NULL) {
    indent(out, indent_level) << "no " << _handle << "\n";
  } else {
    _attrib->write(out, indent_level);
  }
}
