// Filename: stTree.I
// Created by:  drose (06Oct10)
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
//     Function: STTree::get_fullpath
//       Access: Published
//  Description: Returns the full pathname to the SRT file that was
//               loaded for this tree, as passed to the constructor.
////////////////////////////////////////////////////////////////////
INLINE const Filename &STTree::
get_fullpath() const {
  return _fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: STTree::is_valid
//       Access: Published
//  Description: Returns true if the tree was successfully loaded and
//               is ready to be used, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool STTree::
is_valid() const {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: STTree::get_tree
//       Access: Public
//  Description: Returns a const pointer to the internal SpeedTree
//               object.
////////////////////////////////////////////////////////////////////
INLINE const SpeedTree::CTreeRender *STTree::
get_tree() const {
  return &_tree;
}

////////////////////////////////////////////////////////////////////
//     Function: STTree::modify_tree
//       Access: Public
//  Description: Returns a modifiable pointer to the internal SpeedTree
//               object.
////////////////////////////////////////////////////////////////////
INLINE SpeedTree::CTreeRender *STTree::
modify_tree() {
  return &_tree;
}
