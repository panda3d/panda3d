// Filename: qpnodePathComponent.I
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
//     Function: qpNodePathComponent::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpNodePathComponent::CData::
CData() {
  _length = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpNodePathComponent::CData::
CData(const qpNodePathComponent::CData &copy) :
  _next(copy._next),
  _length(copy._length)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::Constructor
//       Access: Private
//  Description: Constructs a new qpNodePathComponent from the
//               indicated node.  Don't try to call this directly; ask
//               the PandaNode to do it for you.
////////////////////////////////////////////////////////////////////
INLINE qpNodePathComponent::
qpNodePathComponent(PandaNode *node, qpNodePathComponent *next) :
  _node(node)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif
  CDWriter cdata(_cycler);
  cdata->_next = next;

  if (next != (qpNodePathComponent *)NULL) {
    cdata->_length = next->get_length() + 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::Copy Constructor
//       Access: Private
//  Description: qpNodePathComponents should not be copied.
////////////////////////////////////////////////////////////////////
INLINE qpNodePathComponent::
qpNodePathComponent(const qpNodePathComponent &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::Copy Assignment Operator
//       Access: Private
//  Description: qpNodePathComponents should not be copied.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePathComponent::
operator = (const qpNodePathComponent &copy) {
  nassertv(false);
}


////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpNodePathComponent::
~qpNodePathComponent() {
  nassertv(_node != (PandaNode *)NULL);
  _node->delete_component(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::get_node
//       Access: Public
//  Description: Returns the node referenced by this component.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *qpNodePathComponent::
get_node() const {
  return _node;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::is_top_node
//       Access: Public
//  Description: Returns true if this component represents the top
//               node in the chain.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePathComponent::
is_top_node() const {
  CDReader cdata(_cycler);
  return (cdata->_next == (qpNodePathComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::is_collapsed
//       Access: Public
//  Description: Returns true if this component has been collapsed
//               with another component.  In this case, the component
//               itself is invalid, and the collapsed component should
//               be used instead.
////////////////////////////////////////////////////////////////////
INLINE bool qpNodePathComponent::
is_collapsed() const {
  CDReader cdata(_cycler);
  return (cdata->_length == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::get_length
//       Access: Public
//  Description: Returns the length of the chain.
////////////////////////////////////////////////////////////////////
INLINE int qpNodePathComponent::
get_length() const {
  CDReader cdata(_cycler);
  nassertr(!is_collapsed(), 0);
  return cdata->_length;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::get_collapsed
//       Access: Public
//  Description: If is_collapsed() returns true, this is the component
//               that this one has been collapsed with, and should be
//               replaced with.
////////////////////////////////////////////////////////////////////
INLINE qpNodePathComponent *qpNodePathComponent::
get_collapsed() const {
  CDReader cdata(_cycler);
  nassertr(is_collapsed(), (qpNodePathComponent *)NULL);
  return cdata->_next;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::set_next
//       Access: Private
//  Description: Sets the next pointer in the chain.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePathComponent::
set_next(qpNodePathComponent *next) {
  CDWriter cdata(_cycler);
  nassertv(!is_collapsed());
  nassertv(next != (qpNodePathComponent *)NULL);
  cdata->_next = next;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::set_top_node
//       Access: Private
//  Description: Severs any connection to the next pointer in the
//               chain and makes this component a top node.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePathComponent::
set_top_node() {
  CDWriter cdata(_cycler);
  nassertv(!is_collapsed());
  cdata->_next = (qpNodePathComponent *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathComponent::collapse_with
//       Access: Private
//  Description: Indicates that this component pointer is no longer
//               valid, and that the indicated component should be
//               used instead.  This is done whenever two
//               qpNodePathComponents have been collapsed together due
//               to an instance being removed higher up in the graph.
////////////////////////////////////////////////////////////////////
INLINE void qpNodePathComponent::
collapse_with(qpNodePathComponent *next) {
  CDWriter cdata(_cycler);
  nassertv(!is_collapsed());
  nassertv(next != (qpNodePathComponent *)NULL);
  cdata->_next = next;
  cdata->_length = 0;

  // We indicate a component has been collapsed by setting its length
  // to zero.
}
