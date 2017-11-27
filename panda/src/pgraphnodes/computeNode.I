/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file computeNode.I
 * @author rdb
 * @date 2014-06-20
 */

/**
 * Adds a dispatch command with the given number of work groups in the X, Y,
 * and Z dimensions.  Any of these values may be set to 1 if the respective
 * dimension should not be used.
 */
INLINE void ComputeNode::
add_dispatch(const LVecBase3i &num_groups) {
  Dispatcher::CDWriter cdata(_dispatcher->_cycler);
  cdata->_dispatches.push_back(num_groups);
}

/**
 * Adds a dispatch command with the given number of work groups in the X, Y,
 * and Z dimensions.  Any of these values may be set to 1 if the respective
 * dimension should not be used.
 */
INLINE void ComputeNode::
add_dispatch(int num_groups_x, int num_groups_y, int num_groups_z) {
  LVecBase3i num_groups(num_groups_x, num_groups_y, num_groups_z);
  add_dispatch(num_groups);
}

/**
 * Removes all dispatch commands.
 */
INLINE void ComputeNode::
clear_dispatches() {
  Dispatcher::CDWriter cdata(_dispatcher->_cycler);
  cdata->_dispatches.clear();
}

/**
 * Returns the number of times add_dispatch has been called on this object.
 */
INLINE size_t ComputeNode::
get_num_dispatches() const {
  Dispatcher::CDReader cdata(_dispatcher->_cycler);
  return cdata->_dispatches.size();
}

/**
 * Returns the group counts of the nth dispatch associated with this object.
 */
INLINE const LVecBase3i &ComputeNode::
get_dispatch(size_t n) const {
  Dispatcher::CDReader cdata(_dispatcher->_cycler);
  nassertr(n < cdata->_dispatches.size(), LVecBase3i::zero());
  return cdata->_dispatches[n];
}

/**
 * Sets the group counts of the nth dispatch associated with this object.
 */
INLINE void ComputeNode::
set_dispatch(size_t n, const LVecBase3i &dispatch) {
  Dispatcher::CDWriter cdata(_dispatcher->_cycler);
  nassertv(n < cdata->_dispatches.size());
  cdata->_dispatches[n] = dispatch;
}

/**
 * Inserts a dispatch command with the given number of work groups in the X,
 * Y, and Z dimensions at the given position in the list of dispatch commands.
 * Any of these values may be set to 1 if the respective dimension should not
 * be used.
 */
INLINE void ComputeNode::
insert_dispatch(size_t n, const LVecBase3i &dispatch) {
  Dispatcher::CDWriter cdata(_dispatcher->_cycler);
  if (n > cdata->_dispatches.size()) {
    n = cdata->_dispatches.size();
  }
  cdata->_dispatches.insert(cdata->_dispatches.begin(), dispatch);
}

/**
 * Erases the given dispatch index from the list.
 */
INLINE void ComputeNode::
remove_dispatch(size_t n) {
  Dispatcher::CDWriter cdata(_dispatcher->_cycler);
  nassertv(n < cdata->_dispatches.size());
  cdata->_dispatches.erase(cdata->_dispatches.begin() + n);
}

/**
 *
 */
INLINE ComputeNode::Dispatcher::CData::
CData() {
}

/**
 *
 */
INLINE ComputeNode::Dispatcher::CData::
CData(const ComputeNode::Dispatcher::CData &copy) :
  _dispatches(copy._dispatches)
{
}
