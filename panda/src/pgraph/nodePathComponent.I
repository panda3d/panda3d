// Filename: nodePathComponent.I
// Created by:  drose (25Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
  // We don't have to bother checking if the component has been
  // collapsed here, since the _node pointer will still be the same
  // even if it has.
  nassertr(_node != (PandaNode *)NULL, _node);
  return _node;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::is_collapsed
//       Access: Public
//  Description: Returns true if this component has been collapsed
//               with another component.  In this case, the component
//               itself is invalid, and the collapsed component should
//               be used instead.
////////////////////////////////////////////////////////////////////
INLINE bool NodePathComponent::
is_collapsed() const {
  CDReader cdata(_cycler);
  return (cdata->_length == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::get_collapsed
//       Access: Public
//  Description: If is_collapsed() returns true, this is the component
//               that this one has been collapsed with, and should be
//               replaced with.
////////////////////////////////////////////////////////////////////
INLINE NodePathComponent *NodePathComponent::
get_collapsed() const {
  nassertr(is_collapsed(), (NodePathComponent *)NULL);
  CDReader cdata(_cycler);
  return cdata->_next;
}
