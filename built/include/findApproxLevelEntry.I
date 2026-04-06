/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file findApproxLevelEntry.I
 * @author drose
 * @date 2002-03-13
 */

/**
 *
 */
INLINE FindApproxLevelEntry::
FindApproxLevelEntry(const WorkingNodePath &node_path, FindApproxPath &approx_path) :
  _node_path(node_path),
  _approx_path(approx_path)
{
  _i = 0;
  _next = nullptr;
  nassertv(_node_path.is_valid());
}

/**
 * This constructor is used to construct the next entry based on a child node
 * of the previous entry's node.
 */
INLINE FindApproxLevelEntry::
FindApproxLevelEntry(const FindApproxLevelEntry &parent,
                     PandaNode *child_node, int i,
                     FindApproxLevelEntry *next) :
  _node_path(parent._node_path, child_node),
  _i(i),
  _approx_path(parent._approx_path),
  _next(next)
{
  nassertv(validate_ptr(this) && validate_ptr(&parent));
  nassertv(_node_path.is_valid());
}

/**
 *
 */
INLINE FindApproxLevelEntry::
FindApproxLevelEntry(const FindApproxLevelEntry &copy) :
  _node_path(copy._node_path),
  _i(copy._i),
  _approx_path(copy._approx_path)
{
  _next = nullptr;
  nassertv(_node_path.is_valid());
}

/**
 *
 */
INLINE void FindApproxLevelEntry::
operator = (const FindApproxLevelEntry &copy) {
  _node_path = copy._node_path;
  _i = copy._i;
  nassertv(&_approx_path == &copy._approx_path);
  nassertv(_node_path.is_valid());
}


/**
 * Returns true if the next node matched by this entry must be a stashed node,
 * false otherwise.
 */
INLINE bool FindApproxLevelEntry::
next_is_stashed(int increment) const {
  return _approx_path.matches_stashed(_i + increment);
}

/**
 * Returns true if this entry represents a solution to the search; i.e.  all
 * the components of the path have been successfully matched.
 */
INLINE bool FindApproxLevelEntry::
is_solution(int increment) const {
  return (_i + increment >= _approx_path.get_num_components());
}
