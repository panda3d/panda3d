/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linkedListNode.I
 * @author drose
 * @date 2006-03-16
 */

/**
 *
 */
INLINE LinkedListNode::
LinkedListNode() {
#ifndef NDEBUG
  _next = nullptr;
  _prev = nullptr;
#endif
}

/**
 * This constructor should be invoked for any LinkedListNodes that will be
 * used to serve as the root of a list.  It sets up the pointers as an empty
 * list.
 */
INLINE LinkedListNode::
LinkedListNode(bool) {
  _next = this;
  _prev = this;
}

/**
 * This move constructor replaces the other link with this one.
 */
INLINE LinkedListNode::
LinkedListNode(LinkedListNode &&from) noexcept {
  if (from._prev != nullptr) {
    nassertv(from._prev->_next == &from);
    from._prev->_next = this;
  }
  _prev = from._prev;
  if (from._next != nullptr) {
    nassertv(from._next->_prev == &from);
    from._next->_prev = this;
  }
  _next = from._next;
  from._next = nullptr;
  from._prev = nullptr;
}

/**
 *
 */
INLINE LinkedListNode::
~LinkedListNode() {
  nassertv((_next == nullptr && _prev == nullptr) || (_next == this && _prev == this));
}

/**
 * Replaces the given other node with this node.
 */
INLINE LinkedListNode &LinkedListNode::
operator = (LinkedListNode &&from) {
  nassertr((_next == nullptr && _prev == nullptr) || (_next == this && _prev == this), *this);
  nassertr(from._prev != nullptr && from._next != nullptr, *this);
  nassertr(from._prev->_next == &from && from._next->_prev == &from, *this);
  from._prev->_next = this;
  from._next->_prev = this;
  _prev = from._prev;
  _next = from._next;
  from._next = nullptr;
  from._prev = nullptr;
  return *this;
}

/**
 * Returns true if the node is member of any list, false if it has been
 * removed or never added.  The head of a list generally appears to to always
 * be a member of itself.
 */
INLINE bool LinkedListNode::
is_on_list() const {
  return (_next != nullptr);
}

/**
 * Removes a LinkedListNode record from the doubly-linked list.
 */
INLINE void LinkedListNode::
remove_from_list() {
  nassertv(_prev != nullptr && _next != nullptr);
  nassertv(_prev->_next == this && _next->_prev == this);
  _prev->_next = _next;
  _next->_prev = _prev;
#ifndef NDEBUG
  _next = nullptr;
  _prev = nullptr;
#endif
}

/**
 * Adds a LinkedListNode record before the indicated node in the doubly-linked
 * list.
 */
INLINE void LinkedListNode::
insert_before(LinkedListNode *node) {
  nassertv(node->_prev != nullptr && node->_prev->_next == node && node->_next->_prev == node);
  nassertv(_prev == nullptr &&
           _next == nullptr);
  _prev = node->_prev;
  _next = node;
  _prev->_next = this;
  node->_prev = this;
}

/**
 * Adds a LinkedListNode record after the indicated node in the doubly-linked
 * list.
 */
INLINE void LinkedListNode::
insert_after(LinkedListNode *node) {
  nassertv(node->_prev != nullptr && node->_prev->_next == node && node->_next->_prev == node);
  nassertv(_prev == nullptr &&
           _next == nullptr);
  _next = node->_next;
  _prev = node;
  _next->_prev = this;
  node->_next = this;
}

/**
 * Given that this LinkedListNode represents the root of a list, and the other
 * pointer represents the root of a different list, move all of the nodes
 * (except the root itself) from other_root onto this list.
 */
INLINE void LinkedListNode::
take_list_from(LinkedListNode *other_root) {
  other_root->_next->_prev = _prev;
  _prev->_next = other_root->_next;
  other_root->_prev->_next = this;
  _prev = other_root->_prev;

  other_root->_next = other_root;
  other_root->_prev = other_root;
}
