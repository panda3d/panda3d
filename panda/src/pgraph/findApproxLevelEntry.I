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
  _next = NULL;
  nassertv(_node_path.is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::Constructor
//       Access: Public
//  Description: This constructor is used to construct the next entry
//               based on a child node of the previous entry's node.
////////////////////////////////////////////////////////////////////
INLINE FindApproxLevelEntry::
FindApproxLevelEntry(const FindApproxLevelEntry &parent,
                     PandaNode *child_node, int i,
                     FindApproxLevelEntry *next) :
  _node_path(parent._node_path, child_node),
  _i(i),
  _approx_path(parent._approx_path),
  _next(next)
{
  nassertv(_node_path.is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FindApproxLevelEntry::
FindApproxLevelEntry(const FindApproxLevelEntry &copy) :
  _node_path(copy._node_path),
  _i(copy._i),
  _approx_path(copy._approx_path)
{
  _next = NULL;
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

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::operator new
//       Access: Public
//  Description: Allocates the memory for a new FindApproxLevelEntry.
//               This is specialized here to provide for fast
//               allocation of these things.
////////////////////////////////////////////////////////////////////
INLINE void *FindApproxLevelEntry::
operator new(size_t size) {
  if (_deleted_chain != (FindApproxLevelEntry *)NULL) {
    FindApproxLevelEntry *obj = _deleted_chain;
    _deleted_chain = _deleted_chain->_next;
    return obj;
  }
#ifndef NDEBUG
  _num_ever_allocated++;
#endif  // NDEBUG
  return ::operator new(size);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::operator delete
//       Access: Public
//  Description: Frees the memory for a deleted FindApproxLevelEntry.
//               This is specialized here to provide for fast
//               allocation of these things.
////////////////////////////////////////////////////////////////////
INLINE void FindApproxLevelEntry::
operator delete(void *ptr) {
  FindApproxLevelEntry *obj = (FindApproxLevelEntry *)ptr;
  obj->_next = _deleted_chain;
  _deleted_chain = obj;
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::get_num_ever_allocated
//       Access: Published, Static
//  Description: Returns the number of FindApproxLevelEntry pointers
//               ever simultaneously allocated; these are now either
//               in active use or have been recycled into the deleted
//               FindApproxLevelEntry pool to be used again.
////////////////////////////////////////////////////////////////////
INLINE int FindApproxLevelEntry::
get_num_ever_allocated() {
  return _num_ever_allocated;
}
