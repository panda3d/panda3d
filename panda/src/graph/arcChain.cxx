// Filename: arcChain.cxx
// Created by:  drose (05Jan01)
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

#include "arcChain.h"
#include "node.h"
#include "namedNode.h"

TypeHandle ArcChain::_type_handle;
TypeHandle ArcChain::ArcComponent::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ArcChain::ArcComponent::
operator = (const ArcComponent &copy) {
  // We have to be careful with the reference counts here,
  // particularly since we might be changing the pointer type from arc
  // pointer to node pointer, or vice-versa, with this assignment.

  // First, ref the new pointer once, so we don't accidentally delete
  // in the interim.
  if (copy.has_arc()) {
    copy._p._arc->ref();
    copy._p._arc->ref_parent();
  } else {
    copy._p._node->ref();
  }

  // Now unref the old pointer.
  if (has_arc()) {
    _p._arc->unref_parent();
    unref_delete(_p._arc);
  } else {
    unref_delete(_p._node);
  }

  // Now reassign the pointers.
  _next = copy._next;
  if (has_arc()) {
    _p._arc = copy._p._arc;
  } else {
    _p._node = copy._p._node;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::r_output
//       Access: Private
//  Description: The recursive implementation of output(), this writes
//               the names of each arc component in order from
//               beginning to end, by first walking to the end of the
//               linked list and then outputting from there.
////////////////////////////////////////////////////////////////////
void ArcChain::
r_output(ostream &out, ArcComponent *comp) const {
  ArcComponent *next = comp->get_next();
  if (next != (ArcComponent *)NULL) {
    // This is not the head of the list; keep going up.
    r_output(out, next);
    out << "/";
  }

  // Now output this component.
  Node *node = comp->get_node();
  if (node->is_of_type(NamedNode::get_class_type())) {
    NamedNode *named_node = DCAST(NamedNode, node);
    if (named_node->has_name()) {
      out << named_node->get_name();
    } else {
      out << node->get_type();
    }
  } else {
    out << node->get_type();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::r_compare_to
//       Access: Private, Static
//  Description: The recursive implementation of compare_to().  Returns
//               < 0 if a sorts before b, > 0 if b sorts before a, or
//               == 0 if they are equivalent.
////////////////////////////////////////////////////////////////////
int ArcChain::
r_compare_to(const ArcComponent *a, const ArcComponent *b) {
  if (a == b) {
    return 0;

  } else if (a == (const ArcComponent *)NULL) {
    return -1;

  } else if (b == (const ArcComponent *)NULL) {
    return 1;

  } else if (a->has_arc() != b->has_arc()) {
    return a->has_arc() - b->has_arc();

  } else if (a->has_arc() && (a->get_arc() != b->get_arc())) {
    return a->get_arc() - b->get_arc();

  } else if (!a->has_arc() && (a->get_node() != b->get_node())) {
    return a->get_node() - b->get_node();

  } else {
    return r_compare_to(a->get_next(), b->get_next());
  }
}

