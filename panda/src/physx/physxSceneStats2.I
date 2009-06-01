// Filename: physxSceneStats2.I
// Created by:  pratt (Jun 20, 2006)
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
//     Function : get_num_stats
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE unsigned int PhysxSceneStats2::
get_num_stats() const {
  return nSceneStats2->numStats;
}

////////////////////////////////////////////////////////////////////
//     Function : get_cur_value
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE int PhysxSceneStats2::
get_cur_value(unsigned int index) const {
  nassertr(index < nSceneStats2->numStats, -1);
  return nSceneStats2->stats[index].curValue;
}

////////////////////////////////////////////////////////////////////
//     Function : get_max_value
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE int PhysxSceneStats2::
get_max_value(unsigned int index) const {
  nassertr(index < nSceneStats2->numStats, -1);
  return nSceneStats2->stats[index].maxValue;
}

////////////////////////////////////////////////////////////////////
//     Function : get_name
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE const char * PhysxSceneStats2::
get_name(unsigned int index) const {
  nassertr(index < nSceneStats2->numStats, "");
  return nSceneStats2->stats[index].name;
}

////////////////////////////////////////////////////////////////////
//     Function : get_parent
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE unsigned int PhysxSceneStats2::
get_parent(unsigned int index) const {
  nassertr(index < nSceneStats2->numStats, 0);
  return nSceneStats2->stats[index].parent;
}

