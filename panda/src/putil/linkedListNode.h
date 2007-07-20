// Filename: linkedListNode.h
// Created by:  drose (16Mar06)
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

#ifndef LINKEDLISTNODE_H
#define LINKEDLISTNODE_H

#include "pandabase.h"
#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//       Class : LinkedListNode
// Description : This just stores the pointers to implement a
//               doubly-linked list of some kind of object.  There are
//               occasions when a hand-rolled linked list is more
//               appropriate than an STL container.
//
//               Typically, each node of the linked list, as well as
//               the root of the list, will inherit from this class.
//
//               Note that this class is not inherently thread-safe;
//               derived classes are responsible for protecting any
//               calls into it within mutexes, if necessary.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL LinkedListNode {
protected:
  INLINE LinkedListNode();
  INLINE LinkedListNode(bool);
  INLINE ~LinkedListNode();

  INLINE bool is_on_list() const;
  INLINE void remove_from_list();
  INLINE void insert_before(LinkedListNode *node);
  INLINE void insert_after(LinkedListNode *node);

  INLINE void take_list_from(LinkedListNode *other_root);

  LinkedListNode *_prev, *_next;
};

#include "linkedListNode.I"

#endif
