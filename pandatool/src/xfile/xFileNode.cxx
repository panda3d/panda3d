// Filename: xFileNode.cxx
// Created by:  drose (03Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "xFileNode.h"

TypeHandle XFileNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: XFileNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileNode::
XFileNode(const string &name) : Namable(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
XFileNode::
~XFileNode() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNode::find_child
//       Access: Public
//  Description: Returns the child with the indicated name, if any, or
//               NULL if none.
////////////////////////////////////////////////////////////////////
XFileNode *XFileNode::
find_child(const string &name) const {
  ChildrenByName::const_iterator ni;
  ni = _children_by_name.find(name);
  if (ni != _children_by_name.end()) {
    return (*ni).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNode::add_child
//       Access: Public, Virtual
//  Description: Adds the indicated node as a child of this node.
////////////////////////////////////////////////////////////////////
void XFileNode::
add_child(XFileNode *node) {
  _children.push_back(node);
  if (node->has_name()) {
    _children_by_name.insert(ChildrenByName::value_type(node->get_name(), node));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNode::clear
//       Access: Public, Virtual
//  Description: Removes all children from the node, and otherwise
//               resets it to its initial state.
////////////////////////////////////////////////////////////////////
void XFileNode::
clear() {
  _children.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNode::write_text
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileNode::
write_text(ostream &out, int indent_level) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write_text(out, indent_level);
  }
}
