/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file selectiveChildNode.cxx
 * @author drose
 * @date 2002-03-06
 */

#include "selectiveChildNode.h"

TypeHandle SelectiveChildNode::_type_handle;


/**
 * Should be overridden by derived classes to return true if this kind of node
 * has some restrictions on the set of children that should be rendered.  Node
 * with this property include LODNodes, SwitchNodes, and SequenceNodes.
 *
 * If this function returns true, get_first_visible_child() and
 * get_next_visible_child() will be called to walk through the list of
 * children during cull, instead of iterating through the entire list.  This
 * method is called after cull_callback(), so cull_callback() may be
 * responsible for the decisions as to which children are visible at the
 * moment.
 */
bool SelectiveChildNode::
has_selective_visibility() const {
  return true;
}

/**
 * Returns the index number of the first visible child of this node, or a
 * number >= get_num_children() if there are no visible children of this node.
 * This is called during the cull traversal, but only if
 * has_selective_visibility() has already returned true.  See
 * has_selective_visibility().
 */
int SelectiveChildNode::
get_first_visible_child() const {
  return _selected_child;
}

/**
 * Returns the index number of the next visible child of this node following
 * the indicated child, or a number >= get_num_children() if there are no more
 * visible children of this node.  See has_selective_visibility() and
 * get_first_visible_child().
 */
int SelectiveChildNode::
get_next_visible_child(int n) const {
  return get_num_children();
}
