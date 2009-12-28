// Filename: physxSceneStats2.cxx
// Created by:  enn0x (20Oct09)
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
//     Function: PhysxSceneStats2::get_num_stats
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE unsigned int PhysxSceneStats2::
get_num_stats() const {

  return _ptr->numStats;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneStats2::get_cur_value
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PhysxSceneStats2::
get_cur_value(unsigned int index) const {

  nassertr(index < _ptr->numStats, -1);
  return _ptr->stats[index].curValue;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneStats2::get_max_value
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PhysxSceneStats2::
get_max_value(unsigned int index) const {

  nassertr(index < _ptr->numStats, -1);
  return _ptr->stats[index].maxValue;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneStats2::get_name
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const char * PhysxSceneStats2::
get_name(unsigned int index) const {

  nassertr(index < _ptr->numStats, "");
  return _ptr->stats[index].name;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneStats2::get_parent
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE unsigned int PhysxSceneStats2::
get_parent(unsigned int index) const {

  nassertr(index < _ptr->numStats, 0);
  return _ptr->stats[index].parent;
}

