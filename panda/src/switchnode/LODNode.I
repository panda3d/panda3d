// Filename: LODNode.I
// Created by:  mike (27Sep99)
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

#include <LOD.h>
#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODNode::
LODNode(const string &name) : SwitchNodeOne(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODNode::
LODNode(const LODNode &copy) :
  SwitchNodeOne(copy),
  _lod(copy._lod)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::add_switch
//       Access: Public
//  Description: Adds a switch range to the LODNode.  This implies
//               that the corresponding child node has been parented
//               to the node.
//
//               The sense of in vs. out distances is as if the object
//               were coming towards you from far away: it switches
//               "in" at the far distance, and switches "out" at the
//               close distance.  Thus, "in" should be larger than
//               "out".
////////////////////////////////////////////////////////////////////
INLINE void LODNode::
add_switch(float in, float out) {
  _lod._switch_vector.push_back(LODSwitch(in, out));
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::set_switch
//       Access: Public
//  Description: Changes the switching range of a particular child of
//               the LODNode.  See add_switch().
////////////////////////////////////////////////////////////////////
INLINE bool LODNode::
set_switch(int index, float in, float out) {
  nassertr(index >= 0 && index < (int)_lod._switch_vector.size(), false);
  _lod._switch_vector[index].set_range(in, out);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::clear_switches
//       Access: Public
//  Description: Removes the set of switching ranges for the LODNode,
//               presumably in conjunction with removing all of its
//               children.  See add_switch().
////////////////////////////////////////////////////////////////////
INLINE void LODNode::
clear_switches(void) {
  _lod._switch_vector.erase(_lod._switch_vector.begin(),
                            _lod._switch_vector.end());
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::get_num_switches
//       Access: Public
//  Description: Returns the number of switch ranges added to the
//               LODNode.  This should correspond to the number of
//               children of the node in order for the LODNode to
//               function correctly.
////////////////////////////////////////////////////////////////////
INLINE int LODNode::
get_num_switches() const {
  return _lod._switch_vector.size();
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::get_in
//       Access: Public
//  Description: Returns the "in" distance of the indicated switch
//               range.  This should be larger than the "out" distance
//               of the same range.
////////////////////////////////////////////////////////////////////
INLINE float LODNode::
get_in(int index) const {
  nassertr(index >= 0 && index < (int)_lod._switch_vector.size(), 0.0);
  return _lod._switch_vector[index].get_in();
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::get_out
//       Access: Public
//  Description: Returns the "out" distance of the indicated switch
//               range.  This should be smaller than the "in" distance
//               of the same range.
////////////////////////////////////////////////////////////////////
INLINE float LODNode::
get_out(int index) const {
  nassertr(index >= 0 && index < (int)_lod._switch_vector.size(), 0.0);
  return _lod._switch_vector[index].get_out();
}
