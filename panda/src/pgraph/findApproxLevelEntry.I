// Filename: findApproxLevelEntry.I
// Created by:  drose (13Mar02)
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


////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FindApproxLevelEntry::
FindApproxLevelEntry(const WorkingNodePath &node_path, FindApproxPath &approx_path) :
  _node_path(node_path),
  _approx_path(approx_path)
{
  _i = 0;
  nassertv(_node_path.is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FindApproxLevelEntry::
FindApproxLevelEntry(const FindApproxLevelEntry &copy, int increment) :
  _node_path(copy._node_path),
  _i(copy._i + increment),
  _approx_path(copy._approx_path)
{
  nassertv(_node_path.is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void FindApproxLevelEntry::
operator = (const FindApproxLevelEntry &copy) {
  _node_path = copy._node_path;
  _i = copy._i;
  nassertv(&_approx_path == &copy._approx_path);
  nassertv(_node_path.is_valid());
}


////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::next_is_stashed
//       Access: Public
//  Description: Returns true if the next node matched by this entry
//               must be a stashed node, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FindApproxLevelEntry::
next_is_stashed(int increment) const {
  return _approx_path.matches_stashed(_i + increment);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::is_solution
//       Access: Public
//  Description: Returns true if this entry represents a solution to
//               the search; i.e. all the components of the path have
//               been successfully matched.
////////////////////////////////////////////////////////////////////
INLINE bool FindApproxLevelEntry::
is_solution(int increment) const {
  return (_i + increment >= _approx_path.get_num_components());
}
