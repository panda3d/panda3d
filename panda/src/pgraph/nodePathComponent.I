// Filename: nodePathComponent.I
// Created by:  drose (25Feb02)
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
//     Function: NodePathComponent::Constructor
//       Access: Private
//  Description: Constructs a new NodePathComponent from the
//               indicated node.  Don't try to call this directly; ask
//               the PandaNode to do it for you.
////////////////////////////////////////////////////////////////////
INLINE NodePathComponent::
NodePathComponent(PandaNode *node, NodePathComponent *next) :
  _node(node),
  _key(0)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif
  CDWriter cdata(_cycler);
  cdata->_next = next;

  if (next != (NodePathComponent *)NULL) {
    cdata->_length = next->get_length() + 1;
  }
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

