// Filename: nodePathComponent.cxx
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

#include "nodePathComponent.h"


// We start the key counters off at 1, since 0 is reserved for an
// empty NodePath (and also for an unassigned key).
int NodePathComponent::_next_key = 1;
TypeHandle NodePathComponent::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *NodePathComponent::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::get_key
//       Access: Public
//  Description: Returns an index number that is guaranteed to be
//               unique for this particular NodePathComponent, and not
//               to be reused for the lifetime of the application
//               (barring integer overflow).
////////////////////////////////////////////////////////////////////
int NodePathComponent::
get_key() const {
  if (_key == 0) {
    // The first time someone asks for a particular component's key,
    // we make it up on the spot.  This helps keep us from wasting
    // index numbers generating a unique number for *every* component
    // in the world (we only have 4.2 billion 32-bit integers, after
    // all)
    ((NodePathComponent *)this)->_key = _next_key++;
  }
  return _key;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::is_top_node
//       Access: Public
//  Description: Returns true if this component represents the top
//               node in the path.
////////////////////////////////////////////////////////////////////
bool NodePathComponent::
is_top_node() const {
  CDReader cdata(_cycler);
  return (cdata->_next == (NodePathComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::get_length
//       Access: Public
//  Description: Returns the length of the path to this node.
////////////////////////////////////////////////////////////////////
int NodePathComponent::
get_length() const {
  CDReader cdata(_cycler);
  return cdata->_length;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::get_next
//       Access: Public
//  Description: Returns the next component in the path.
////////////////////////////////////////////////////////////////////
NodePathComponent *NodePathComponent::
get_next() const {
  CDReader cdata(_cycler);
  NodePathComponent *next = cdata->_next;
  
  return next;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::fix_length
//       Access: Public
//  Description: Checks that the length indicated by the component is
//               one more than the length of its predecessor.  If this
//               is broken, fixes it and returns true indicating the
//               component has been changed; otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool NodePathComponent::
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
//     Function: NodePathComponent::output
//       Access: Public
//  Description: The recursive implementation of NodePath::output(),
//               this writes the names of each node component in order
//               from beginning to end, by first walking to the end of
//               the linked list and then outputting from there.
////////////////////////////////////////////////////////////////////
void NodePathComponent::
output(ostream &out) const {
  PandaNode *node = this->get_node();
  NodePathComponent *next = this->get_next();
  if (next != (NodePathComponent *)NULL) {
    // This is not the head of the list; keep going up.
    next->output(out);
    out << "/";

    PandaNode *parent_node = next->get_node();
    if (parent_node->find_stashed(node) >= 0) {
      // The node is stashed.
      out << "@@";

    } else if (node->find_parent(parent_node) < 0) {
      // Oops, there's an error.  This shouldn't happen.
      out << ".../";
    }
  }

  // Now output this component.
  if (node->has_name()) {
    out << node->get_name();
  } else {
    out << "-" << node->get_type();
  }
  //  out << "[" << this->get_length() << "]";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::set_next
//       Access: Private
//  Description: Sets the next pointer in the path.
////////////////////////////////////////////////////////////////////
void NodePathComponent::
set_next(NodePathComponent *next) {
  nassertv(next != (NodePathComponent *)NULL);
  CDWriter cdata(_cycler);
  cdata->_next = next;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathComponent::set_top_node
//       Access: Private
//  Description: Severs any connection to the next pointer in the
//               path and makes this component a top node.
////////////////////////////////////////////////////////////////////
void NodePathComponent::
set_top_node() {
  CDWriter cdata(_cycler);
  cdata->_next = (NodePathComponent *)NULL;
}
