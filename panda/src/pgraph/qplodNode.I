// Filename: qplodNode.I
// Created by:  drose (06Mar02)
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
//     Function: qpLODNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpLODNode::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpLODNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpLODNode::CData::
CData(const qpLODNode::CData &copy) :
  _lod(copy._lod)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpLODNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE qpLODNode::
qpLODNode(const string &name) :
  SelectiveChildNode(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpLODNode::add_switch
//       Access: Published
//  Description: Adds a switch range to the qpLODNode.  This implies
//               that the corresponding child node has been parented
//               to the node.
//
//               The sense of in vs. out distances is as if the object
//               were coming towards you from far away: it switches
//               "in" at the far distance, and switches "out" at the
//               close distance.  Thus, "in" should be larger than
//               "out".
////////////////////////////////////////////////////////////////////
INLINE void qpLODNode::
add_switch(float in, float out) {
  CDWriter cdata(_cycler);
  cdata->_lod._switch_vector.push_back(LODSwitch(in, out));
}

////////////////////////////////////////////////////////////////////
//     Function: qpLODNode::set_switch
//       Access: Published
//  Description: Changes the switching range of a particular child of
//               the qpLODNode.  See add_switch().
////////////////////////////////////////////////////////////////////
INLINE bool qpLODNode::
set_switch(int index, float in, float out) {
  CDWriter cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_lod._switch_vector.size(), false);
  cdata->_lod._switch_vector[index].set_range(in, out);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpLODNode::clear_switches
//       Access: Published
//  Description: Removes the set of switching ranges for the qpLODNode,
//               presumably in conjunction with removing all of its
//               children.  See add_switch().
////////////////////////////////////////////////////////////////////
INLINE void qpLODNode::
clear_switches(void) {
  CDWriter cdata(_cycler);
  cdata->_lod._switch_vector.erase(cdata->_lod._switch_vector.begin(),
                                   cdata->_lod._switch_vector.end());
}

////////////////////////////////////////////////////////////////////
//     Function: qpLODNode::get_num_switches
//       Access: Published
//  Description: Returns the number of switch ranges added to the
//               qpLODNode.  This should correspond to the number of
//               children of the node in order for the qpLODNode to
//               function correctly.
////////////////////////////////////////////////////////////////////
INLINE int qpLODNode::
get_num_switches() const {
  CDReader cdata(_cycler);
  return cdata->_lod._switch_vector.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpLODNode::get_in
//       Access: Published
//  Description: Returns the "in" distance of the indicated switch
//               range.  This should be larger than the "out" distance
//               of the same range.
////////////////////////////////////////////////////////////////////
INLINE float qpLODNode::
get_in(int index) const {
  CDReader cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_lod._switch_vector.size(), 0.0);
  return cdata->_lod._switch_vector[index].get_in();
}

////////////////////////////////////////////////////////////////////
//     Function: qpLODNode::get_out
//       Access: Published
//  Description: Returns the "out" distance of the indicated switch
//               range.  This should be smaller than the "in" distance
//               of the same range.
////////////////////////////////////////////////////////////////////
INLINE float qpLODNode::
get_out(int index) const {
  CDReader cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_lod._switch_vector.size(), 0.0);
  return cdata->_lod._switch_vector[index].get_out();
}

////////////////////////////////////////////////////////////////////
//     Function: qpLODNode::set_center
//       Access: Published
//  Description: Specifies the center of the LOD.  This is the point
//               that is compared to the camera (in camera space) to
//               determine the particular LOD that should be chosen.
////////////////////////////////////////////////////////////////////
INLINE void qpLODNode::
set_center(const LPoint3f &center) {
  CDWriter cdata(_cycler);
  cdata->_lod._center = center;
}

////////////////////////////////////////////////////////////////////
//     Function: qpLODNode::get_center
//       Access: Published
//  Description: Returns the center of the LOD.  This is the point
//               that is compared to the camera (in camera space) to
//               determine the particular LOD that should be chosen.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &qpLODNode::
get_center() const {
  CDReader cdata(_cycler);
  return cdata->_lod._center;
}
