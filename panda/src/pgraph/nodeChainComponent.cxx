// Filename: nodeChainComponent.cxx
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

#include "nodeChainComponent.h"

TypeHandle NodeChainComponent::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: NodeChainComponent::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *NodeChainComponent::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChainComponent::get_next
//       Access: Public
//  Description: Returns the next component in the chain.
////////////////////////////////////////////////////////////////////
NodeChainComponent *NodeChainComponent::
get_next() const {
  CDReader cdata(_cycler);
  nassertr(!is_collapsed(), (NodeChainComponent *)NULL);

  NodeChainComponent *next = cdata->_next;

  // If the next component has been collapsed, transparently update
  // the pointer to get the actual node, and store the new pointer,
  // before we return.  Collapsing can happen at any time to any
  // component in the chain and we have to deal with it.
  if (next != (NodeChainComponent *)NULL && next->is_collapsed()) {
    next = next->uncollapse();
    ((NodeChainComponent *)this)->set_next(next);
  }

  return next;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChainComponent::fix_length
//       Access: Public
//  Description: Checks that the length indicated by the component is
//               one more than the length of its predecessor.  If this
//               is broken, fixes it and returns true indicating the
//               component has been changed; otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool NodeChainComponent::
fix_length() {
  int length_should_be = 1;
  if (!is_top_node()) {
    length_should_be = get_next()->get_length() + 1;
  }
  if (get_length() == length_should_be) {
    return false;
  }

  CDWriter cdata(_cycler);
  cdata->_length = length_should_be;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChainComponent::uncollapse
//       Access: Public
//  Description: Returns this component pointer if the component is
//               not collapsed; or if it has been collapsed, returns
//               the pointer it has been collapsed into.
//
//               Collapsing can happen at any time to any component in
//               the chain and we have to deal with it.  It happens
//               when a node is removed further up the chain that
//               results in two instances becoming the same thing.
////////////////////////////////////////////////////////////////////
NodeChainComponent *NodeChainComponent::
uncollapse() {
  NodeChainComponent *comp = this;

  while (comp->is_collapsed()) {
    comp = comp->get_collapsed();
  }

  return comp;
}
