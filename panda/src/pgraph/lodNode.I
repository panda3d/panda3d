// Filename: lodNode.I
// Created by:  drose (06Mar02)
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
//     Function: LODNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODNode::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODNode::CData::
CData(const LODNode::CData &copy) :
  _lod(copy._lod)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODNode::
LODNode(const string &name) :
  SelectiveChildNode(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODNode::
LODNode(const LODNode &copy) :
  SelectiveChildNode(copy),
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::add_switch
//       Access: Published
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
  CDWriter cdata(_cycler);
  cdata->_lod._switch_vector.push_back(LODSwitch(in, out));
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::set_switch
//       Access: Published
//  Description: Changes the switching range of a particular child of
//               the LODNode.  See add_switch().
////////////////////////////////////////////////////////////////////
INLINE bool LODNode::
set_switch(int index, float in, float out) {
  CDWriter cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_lod._switch_vector.size(), false);
  cdata->_lod._switch_vector[index].set_range(in, out);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::clear_switches
//       Access: Published
//  Description: Removes the set of switching ranges for the LODNode,
//               presumably in conjunction with removing all of its
//               children.  See add_switch().
////////////////////////////////////////////////////////////////////
INLINE void LODNode::
clear_switches(void) {
  CDWriter cdata(_cycler);
  cdata->_lod._switch_vector.erase(cdata->_lod._switch_vector.begin(),
                                   cdata->_lod._switch_vector.end());
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::get_num_switches
//       Access: Published
//  Description: Returns the number of switch ranges added to the
//               LODNode.  This should correspond to the number of
//               children of the node in order for the LODNode to
//               function correctly.
////////////////////////////////////////////////////////////////////
INLINE int LODNode::
get_num_switches() const {
  CDReader cdata(_cycler);
  return cdata->_lod._switch_vector.size();
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::get_in
//       Access: Published
//  Description: Returns the "in" distance of the indicated switch
//               range.  This should be larger than the "out" distance
//               of the same range.
////////////////////////////////////////////////////////////////////
INLINE float LODNode::
get_in(int index) const {
  CDReader cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_lod._switch_vector.size(), 0.0);
  return cdata->_lod._switch_vector[index].get_in();
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::get_out
//       Access: Published
//  Description: Returns the "out" distance of the indicated switch
//               range.  This should be smaller than the "in" distance
//               of the same range.
////////////////////////////////////////////////////////////////////
INLINE float LODNode::
get_out(int index) const {
  CDReader cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_lod._switch_vector.size(), 0.0);
  return cdata->_lod._switch_vector[index].get_out();
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::set_center
//       Access: Published
//  Description: Specifies the center of the LOD.  This is the point
//               that is compared to the camera (in camera space) to
//               determine the particular LOD that should be chosen.
////////////////////////////////////////////////////////////////////
INLINE void LODNode::
set_center(const LPoint3f &center) {
  CDWriter cdata(_cycler);
  cdata->_lod._center = center;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::get_center
//       Access: Published
//  Description: Returns the center of the LOD.  This is the point
//               that is compared to the camera (in camera space) to
//               determine the particular LOD that should be chosen.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &LODNode::
get_center() const {
  CDReader cdata(_cycler);
  return cdata->_lod._center;
}
