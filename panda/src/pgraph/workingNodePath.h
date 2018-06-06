/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file workingNodePath.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef WORKINGNODEPATH_H
#define WORKINGNODEPATH_H

#include "pandabase.h"

#include "nodePath.h"
#include "nodePathComponent.h"

/**
 * This is a class designed to support low-overhead traversals of the complete
 * scene graph, with a memory of the complete path through the graph at any
 * given point.
 *
 * You could just use a regular NodePath to do this, but since the NodePath
 * requires storing NodePathComponents on each node as it is constructed, and
 * then removing them when it destructs, there is considerable overhead in
 * that approach.
 *
 * The WorkingNodePath eliminates this overhead (but does not guarantee
 * consistency if the scene graph changes while the path is held).
 *
 * At any given point, you may ask the WorkingNodePath for its actual
 * NodePath, and it will construct and return a new NodePath representing the
 * complete generated chain.
 */
class EXPCL_PANDA_PGRAPH WorkingNodePath {
public:
  INLINE WorkingNodePath(const NodePath &start);
  INLINE WorkingNodePath(const WorkingNodePath &copy);
  INLINE WorkingNodePath(const WorkingNodePath &parent, PandaNode *child);
  INLINE ~WorkingNodePath();

  INLINE void operator = (const WorkingNodePath &copy);

  bool is_valid() const;

  INLINE NodePath get_node_path() const;
  INLINE PandaNode *node() const;

  int get_num_nodes() const;
  PandaNode *get_node(int index) const;

  void output(std::ostream &out) const;

PUBLISHED:
  MAKE_PROPERTY(valid, is_valid);
  MAKE_PROPERTY(node_path, get_node_path);

private:
  PT(NodePathComponent) r_get_node_path() const;

  // Either one or the other of these pointers will be filled in, but never
  // both.  We maintain a linked list of WorkingNodePath objects, with a
  // NodePathComponent at the head of the list.
  const WorkingNodePath *_next;
  PT(NodePathComponent) _start;

  PT(PandaNode) _node;
};

INLINE std::ostream &operator << (std::ostream &out, const WorkingNodePath &node_path);

#include "workingNodePath.I"

#endif
