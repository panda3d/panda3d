// Filename: findApproxLevel.I
// Created by:  drose (18Feb00)
// 
////////////////////////////////////////////////////////////////////

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FindApproxLevelEntry::
FindApproxLevelEntry(const NodePath &node_path, FindApproxPath &approx_path) :
  _node_path(node_path),
  _approx_path(approx_path) 
{
  _i = 0;
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
}


////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::next_is_stashed
//       Access: Public
//  Description: Returns true if the next node matched by this entry
//               must be a stashed node, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FindApproxLevelEntry::
next_is_stashed() const {
  return _approx_path.matches_stashed(_i);
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::is_solution
//       Access: Public
//  Description: Returns true if this entry represents a solution to
//               the search; i.e. all the components of the path have
//               been successfully matched.
////////////////////////////////////////////////////////////////////
INLINE bool FindApproxLevelEntry::
is_solution() const {
  return (_i >= _approx_path.get_num_components());
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevel::add_entry
//       Access: Public
//  Description: Adds a new entry to the level.
////////////////////////////////////////////////////////////////////
INLINE void FindApproxLevel::
add_entry(const FindApproxLevelEntry &entry) {
  _v.push_back(entry);
}

