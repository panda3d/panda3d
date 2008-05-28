// Filename: linkedListNode.I
// Created by:  drose (16Mar06)
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
//     Function: LinkedListNode::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE LinkedListNode::
LinkedListNode() {
#ifndef NDEBUG
  _next = NULL;
  _prev = NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LinkedListNode::Constructor 2
//       Access: Protected
//  Description: This constructor should be invoked for any
//               LinkedListNodes that will be used to serve as the
//               root of a list.  It sets up the pointers as an empty
//               list.
////////////////////////////////////////////////////////////////////
INLINE LinkedListNode::
LinkedListNode(bool) {
  _next = this;
  _prev = this;
}

////////////////////////////////////////////////////////////////////
//     Function: LinkedListNode::Destructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE LinkedListNode::
~LinkedListNode() {
  nassertv((_next == NULL && _prev == NULL) || (_next == this && _prev == this));
}

////////////////////////////////////////////////////////////////////
//     Function: LinkedListNode::is_on_list
//       Access: Protected
//  Description: Returns true if the node is member of any list, false
//               if it has been removed or never added.  The head of a
//               list generally appears to to always be a member of
//               itself.
////////////////////////////////////////////////////////////////////
INLINE bool LinkedListNode::
is_on_list() const {
  return (_next != NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: LinkedListNode::remove_from_list
//       Access: Protected
//  Description: Removes a LinkedListNode record from the
//               doubly-linked list.
////////////////////////////////////////////////////////////////////
INLINE void LinkedListNode::
remove_from_list() {
  nassertv(_prev != NULL && _next != NULL);
  nassertv(_prev->_next == this && _next->_prev == this);
  _prev->_next = _next;
  _next->_prev = _prev;
#ifndef NDEBUG
  _next = NULL;
  _prev = NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LinkedListNode::insert_before
//       Access: Protected
//  Description: Adds a LinkedListNode record before the indicated
//               node in the doubly-linked list.
////////////////////////////////////////////////////////////////////
INLINE void LinkedListNode::
insert_before(LinkedListNode *node) {
  nassertv(node->_prev != NULL && node->_prev->_next == node && node->_next->_prev == node);
  nassertv(_prev == (LinkedListNode *)NULL &&
           _next == (LinkedListNode *)NULL);
  _prev = node->_prev;
  _next = node;
  _prev->_next = this;
  node->_prev = this;
}

////////////////////////////////////////////////////////////////////
//     Function: LinkedListNode::insert_after
//       Access: Protected
//  Description: Adds a LinkedListNode record after the indicated
//               node in the doubly-linked list.
////////////////////////////////////////////////////////////////////
INLINE void LinkedListNode::
insert_after(LinkedListNode *node) {
  nassertv(node->_prev != NULL && node->_prev->_next == node && node->_next->_prev == node);
  nassertv(_prev == (LinkedListNode *)NULL &&
           _next == (LinkedListNode *)NULL);
  _next = node->_next;
  _prev = node;
  _next->_prev = this;
  node->_next = this;
}

////////////////////////////////////////////////////////////////////
//     Function: LinkedListNode::take_list_from
//       Access: Protected
//  Description: Given that this LinkedListNode represents the root of
//               a list, and the other pointer represents the root of
//               a different list, move all of the nodes (except the
//               root itself) from other_root onto this list.
////////////////////////////////////////////////////////////////////
INLINE void LinkedListNode::
take_list_from(LinkedListNode *other_root) {
  other_root->_next->_prev = _prev;
  _prev->_next = other_root->_next;
  other_root->_prev->_next = this;
  _prev = other_root->_prev;

  other_root->_next = other_root;
  other_root->_prev = other_root;
}
