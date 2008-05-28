// Filename: nodePathComponent.I
// Created by:  drose (25Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NodePathComponent::CData::
CData() {
  _length = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NodePathComponent::CData::
CData(const NodePathComponent::CData &copy) :
  _next(copy._next),
  _length(copy._length)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::Copy Constructor
//       Access: Private
//  Description: NodePathComponents should not be copied.
////////////////////////////////////////////////////////////////////
INLINE NodePathComponent::
NodePathComponent(const NodePathComponent &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::Copy Assignment Operator
//       Access: Private
//  Description: NodePathComponents should not be copied.
////////////////////////////////////////////////////////////////////
INLINE void NodePathComponent::
operator = (const NodePathComponent &copy) {
  nassertv(false);
}


////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NodePathComponent::
~NodePathComponent() {
  nassertv(_node != (PandaNode *)NULL);
  _node->delete_component(this);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::get_node
//       Access: Public
//  Description: Returns the node referenced by this component.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *NodePathComponent::
get_node() const {
  nassertr(_node != (PandaNode *)NULL, _node);
  return _node;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::has_key
//       Access: Public
//  Description: Returns true if the key for this component has
//               already been generated, false otherwise.  Even if
//               this returns false, calling get_key() will still
//               return a valid key; that will simply cause the key to
//               be generated on-the-fly.
////////////////////////////////////////////////////////////////////
INLINE bool NodePathComponent::
has_key() const {
  return (_key != 0);
}

INLINE ostream &operator << (ostream &out, const NodePathComponent &comp) {
  comp.output(out);
  return out;
}

