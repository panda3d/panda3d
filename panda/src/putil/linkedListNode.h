/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linkedListNode.h
 * @author drose
 * @date 2006-03-16
 */

#ifndef LINKEDLISTNODE_H
#define LINKEDLISTNODE_H

#include "pandabase.h"
#include "pnotify.h"

/**
 * This just stores the pointers to implement a doubly-linked list of some
 * kind of object.  There are occasions when a hand-rolled linked list is more
 * appropriate than an STL container.
 *
 * Typically, each node of the linked list, as well as the root of the list,
 * will inherit from this class.
 *
 * Note that this class is not inherently thread-safe; derived classes are
 * responsible for protecting any calls into it within mutexes, if necessary.
 */
class EXPCL_PANDA_PUTIL LinkedListNode {
protected:
  INLINE LinkedListNode();
  INLINE LinkedListNode(bool);
  INLINE LinkedListNode(LinkedListNode &&from) noexcept;
  INLINE ~LinkedListNode();

  INLINE LinkedListNode &operator = (LinkedListNode &&from);

  INLINE bool is_on_list() const;
  INLINE void remove_from_list();
  INLINE void insert_before(LinkedListNode *node);
  INLINE void insert_after(LinkedListNode *node);

  INLINE void take_list_from(LinkedListNode *other_root);

  LinkedListNode *_prev, *_next;
};

#include "linkedListNode.I"

#endif
