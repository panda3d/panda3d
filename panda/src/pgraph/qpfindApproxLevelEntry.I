// Filename: qpfindApproxLevelEntry.I
// Created by:  drose (13Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxLevelEntry::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpFindApproxLevelEntry::
qpFindApproxLevelEntry(const qpNodePath &node_path, qpFindApproxPath &approx_path) :
  _node_path(node_path),
  _approx_path(approx_path)
{
  _i = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxLevelEntry::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpFindApproxLevelEntry::
qpFindApproxLevelEntry(const qpFindApproxLevelEntry &copy) :
  _node_path(copy._node_path),
  _i(copy._i),
  _approx_path(copy._approx_path)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxLevelEntry::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void qpFindApproxLevelEntry::
operator = (const qpFindApproxLevelEntry &copy) {
  _node_path = copy._node_path;
  _i = copy._i;
  nassertv(&_approx_path == &copy._approx_path);
}


////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxLevelEntry::next_is_stashed
//       Access: Public
//  Description: Returns true if the next node matched by this entry
//               must be a stashed node, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpFindApproxLevelEntry::
next_is_stashed() const {
  return _approx_path.matches_stashed(_i);
}

////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxLevelEntry::is_solution
//       Access: Public
//  Description: Returns true if this entry represents a solution to
//               the search; i.e. all the components of the path have
//               been successfully matched.
////////////////////////////////////////////////////////////////////
INLINE bool qpFindApproxLevelEntry::
is_solution() const {
  return (_i >= _approx_path.get_num_components());
}
