// Filename: computeNode.I
// Created by:  rdb (20Jun14)
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
//     Function: ComputeNode::add_dispatch
//       Access: Published
//  Description: Adds a dispatch command with the given number of
//               work groups in the X, Y, and Z dimensions.  Any
//               of these values may be set to 1 if the respective
//               dimension should not be used.
////////////////////////////////////////////////////////////////////
INLINE void ComputeNode::
add_dispatch(const LVecBase3i &num_groups) {
  Dispatcher::CDWriter cdata(_dispatcher->_cycler);
  cdata->_dispatches.push_back(num_groups);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::add_dispatch
//       Access: Published
//  Description: Adds a dispatch command with the given number of
//               work groups in the X, Y, and Z dimensions.  Any
//               of these values may be set to 1 if the respective
//               dimension should not be used.
////////////////////////////////////////////////////////////////////
INLINE void ComputeNode::
add_dispatch(int num_groups_x, int num_groups_y, int num_groups_z) {
  LVecBase3i num_groups(num_groups_x, num_groups_y, num_groups_z);
  add_dispatch(num_groups);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::clear_dispatches
//       Access: Published
//  Description: Removes all dispatch commands.
////////////////////////////////////////////////////////////////////
INLINE void ComputeNode::
clear_dispatches() {
  Dispatcher::CDWriter cdata(_dispatcher->_cycler);
  cdata->_dispatches.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::get_num_dispatches
//       Access: Published
//  Description: Returns the number of times add_dispatch has been
//               called on this object.
////////////////////////////////////////////////////////////////////
INLINE int ComputeNode::
get_num_dispatches() const {
  Dispatcher::CDReader cdata(_dispatcher->_cycler);
  return cdata->_dispatches.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::get_dispatch
//       Access: Published
//  Description: Returns the group counts of the nth dispatch
//               associated with this object.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3i &ComputeNode::
get_dispatch(int n) const {
  Dispatcher::CDReader cdata(_dispatcher->_cycler);
  nassertr(n >= 0 && (size_t)n < cdata->_dispatches.size(), LVecBase3i::zero());
  return cdata->_dispatches[n];
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::Dispatcher::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputeNode::Dispatcher::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::Dispatcher::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputeNode::Dispatcher::CData::
CData(const ComputeNode::Dispatcher::CData &copy) :
  _dispatches(copy._dispatches)
{
}
