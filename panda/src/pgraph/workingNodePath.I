/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file workingNodePath.I
 * @author drose
 * @date 2002-03-16
 */

/**
 * Creates a WorkingNodePath that is the same as the indicated NodePath.  This
 * is generally used to begin the traversal of a scene graph with the root
 * NodePath.
 */
INLINE WorkingNodePath::
WorkingNodePath(const NodePath &start) {
  nassertv(!start.is_empty());
  _next = nullptr;
  _start = start._head;
  _node = start.node();
}

/**
 *
 */
INLINE WorkingNodePath::
WorkingNodePath(const WorkingNodePath &copy) :
  _next(copy._next),
  _start(copy._start),
  _node(copy._node)
{
  nassertv(_next != nullptr ||
           _start != nullptr);
}

/**
 * Creates a WorkingNodePath that is the same as the indicated
 * WorkingNodePath, plus one node.  This is generally used to continue the
 * traversal to the next node.
 */
INLINE WorkingNodePath::
WorkingNodePath(const WorkingNodePath &parent, PandaNode *child) :
  _next(&parent),
  _start(nullptr),
  _node(child) {
  nassertv(_node != _next->_node);
}

/**
 *
 */
INLINE WorkingNodePath::
~WorkingNodePath() {
}

/**
 *
 */
INLINE void WorkingNodePath::
operator = (const WorkingNodePath &copy) {
  _next = copy._next;
  _start = copy._start;
  _node = copy._node;

  nassertv(_next != nullptr ||
           _start != nullptr);
}

/**
 * Constructs and returns an actual NodePath that represents the same path we
 * have just traversed.
 */
INLINE NodePath WorkingNodePath::
get_node_path() const {
  NodePath result;
  result._head = r_get_node_path();
  nassertr(result._head != nullptr, NodePath::fail());
  return result;
}

/**
 * Returns the node traversed to so far.
 */
INLINE PandaNode *WorkingNodePath::
node() const {
  return _node;
}

INLINE std::ostream &
operator << (std::ostream &out, const WorkingNodePath &node_path) {
  node_path.output(out);
  return out;
}
