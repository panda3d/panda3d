// Filename: fadelodNode.I
// Created by:  sshodhan (14Jun04)
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
//     Function: FadeLODNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FadeLODNode::CData::
CData() {
  _fade_time = 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FadeLODNode::CData::
CData(const FadeLODNode::CData &copy) :
  _lod(copy._lod) {
  _fade_time = copy._fade_time;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FadeLODNode::
FadeLODNode(const string &name) :
  PandaNode(name) {
  _fade_mode = false;
  _previous_child = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FadeLODNode::
FadeLODNode(const FadeLODNode &copy) :
  PandaNode(copy),
  _cycler(copy._cycler) {
  _fade_mode = copy._fade_mode;
  _previous_child = copy._previous_child;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::add_switch
//       Access: Published
//  Description: Adds a switch range to the FadeLODNode.  This implies
//               that the corresponding child node has been parented
//               to the node.
//
//               The sense of in vs. out distances is as if the object
//               were coming towards you from far away: it switches
//               "in" at the far distance, and switches "out" at the
//               close distance.  Thus, "in" should be larger than
//               "out".
////////////////////////////////////////////////////////////////////
INLINE void FadeLODNode::
add_switch(float in, float out) {
  CDWriter cdata(_cycler);
  cdata->_lod._switch_vector.push_back(LODSwitch(in, out));
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::set_switch
//       Access: Published
//  Description: Changes the switching range of a particular child of
//               the LODNode.  See add_switch().
////////////////////////////////////////////////////////////////////
INLINE bool FadeLODNode::
set_switch(int index, float in, float out) {
  CDWriter cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_lod._switch_vector.size(), false);
  cdata->_lod._switch_vector[index].set_range(in, out);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::clear_switches
//       Access: Published
//  Description: Removes the set of switching ranges for the LODNode,
//               presumably in conjunction with removing all of its
//               children.  See add_switch().
////////////////////////////////////////////////////////////////////
INLINE void FadeLODNode::
clear_switches(void) {
  CDWriter cdata(_cycler);
  cdata->_lod._switch_vector.erase(cdata->_lod._switch_vector.begin(),
                                   cdata->_lod._switch_vector.end());
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::get_num_switches
//       Access: Published
//  Description: Returns the number of switch ranges added to the
//               LODNode.  This should correspond to the number of
//               children of the node in order for the FadeLODNode to
//               function correctly.
////////////////////////////////////////////////////////////////////
INLINE int FadeLODNode::
get_num_switches() const {
  CDReader cdata(_cycler);
  return cdata->_lod._switch_vector.size();
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::get_in
//       Access: Published
//  Description: Returns the "in" distance of the indicated switch
//               range.  This should be larger than the "out" distance
//               of the same range.
////////////////////////////////////////////////////////////////////
INLINE float FadeLODNode::
get_in(int index) const {
  CDReader cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_lod._switch_vector.size(), 0.0);
  return cdata->_lod._switch_vector[index].get_in();
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::get_out
//       Access: Published
//  Description: Returns the "out" distance of the indicated switch
//               range.  This should be smaller than the "in" distance
//               of the same range.
////////////////////////////////////////////////////////////////////
INLINE float FadeLODNode::
get_out(int index) const {
  CDReader cdata(_cycler);
  nassertr(index >= 0 && index < (int)cdata->_lod._switch_vector.size(), 0.0);
  return cdata->_lod._switch_vector[index].get_out();
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::set_center
//       Access: Published
//  Description: Specifies the center of the LOD.  This is the point
//               that is compared to the camera (in camera space) to
//               determine the particular LOD that should be chosen.
////////////////////////////////////////////////////////////////////
INLINE void FadeLODNode::
set_center(const LPoint3f &center) {
  CDWriter cdata(_cycler);
  cdata->_lod._center = center;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::get_center
//       Access: Published
//  Description: Returns the center of the LOD.  This is the point
//               that is compared to the camera (in camera space) to
//               determine the particular LOD that should be chosen.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &FadeLODNode::
get_center() const {
  CDReader cdata(_cycler);
  return cdata->_lod._center;
}


////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::set_fade_time
//       Access: Published
//  Description: set the time taken to complete an LOD switch
////////////////////////////////////////////////////////////////////
INLINE FadeLODNode::
set_fade_time(float t) {
  CDWriter cdata(_cycler);
  cdata->_fade_time = t;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::get_fade_time
//       Access: Published
//  Description: get the time taken to complete an LOD switch
////////////////////////////////////////////////////////////////////
INLINE float FadeLODNode::
get_fade_time() const {
  CDReader cdata(_cycler);
  return cdata->_fade_time;
}

