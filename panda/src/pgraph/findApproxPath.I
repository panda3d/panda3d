/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file findApproxPath.I
 * @author drose
 * @date 2002-03-13
 */

/**
 *
 */
INLINE FindApproxPath::
FindApproxPath() {
  _return_hidden = true;
  _return_stashed = false;
  _case_insensitive = false;
}

/**
 * Returns the number of components in the path.
 */
INLINE int FindApproxPath::
get_num_components() const {
  return _path.size();
}

/**
 * Returns true if the nth component is of type match_many, which will require
 * special handling.
 */
INLINE bool FindApproxPath::
is_component_match_many(int index) const {
  nassertr(index >= 0 && index < (int)_path.size(), false);
  return (_path[index]._type == CT_match_many);
}

/**
 * Returns true if the nth component of the path matches the indicated node,
 * false otherwise.
 */
INLINE bool FindApproxPath::
matches_component(int index, PandaNode *node) const {
  nassertr(index >= 0 && index < (int)_path.size(), false);
  return (_path[index].matches(node));
}

/**
 * Returns true if the nth component of the path matches a stashed node only,
 * false otherwise.
 */
INLINE bool FindApproxPath::
matches_stashed(int index) const {
  if (index >= 0 && index < (int)_path.size()) {
    return ((_path[index]._flags & CF_stashed) != 0);
  } else {
    return false;
  }
}

/**
 * Returns true if this path allows returning of hidden nodes, false
 * otherwise.
 */
INLINE bool FindApproxPath::
return_hidden() const {
  return _return_hidden;
}

/**
 * Returns true if this path allows returning of stashed nodes, false
 * otherwise.
 */
INLINE bool FindApproxPath::
return_stashed() const {
  return _return_stashed;
}

/**
 * Returns true if the search is case-insensitive, false if it is case-
 * sensitive.
 */
INLINE bool FindApproxPath::
case_insensitive() const {
  return _case_insensitive;
}

/**
 * Formats the nth component of the path to the indicated output stream.
 */
INLINE void FindApproxPath::
output_component(std::ostream &out, int index) const {
  nassertv(index >= 0 && index < (int)_path.size());
  out << _path[index];
}
