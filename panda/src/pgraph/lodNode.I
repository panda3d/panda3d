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
CData() :
  _center(0.0f, 0.0f, 0.0f),
  _lowest(0),
  _highest(0),
  _got_force_switch(false),
  _force_switch(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODNode::CData::
CData(const LODNode::CData &copy) :
  _center(copy._center),
  _switch_vector(copy._switch_vector),
  _lowest(copy._lowest),
  _highest(copy._highest),
  _got_force_switch(copy._got_force_switch),
  _force_switch(copy._force_switch)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Switch::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODNode::Switch::
Switch(float in, float out) {
  set_range(in, out);
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Switch::get_in
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float LODNode::Switch::
get_in() const {
  return _in;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Switch::get_out
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float LODNode::Switch::
get_out() const {
  return _out;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Switch::set_range
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LODNode::Switch::
set_range(float in, float out) {
  _in = in;
  _out = out;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Switch::in_range
//       Access: Public
//  Description: Computes the distance between two points and returns
//               true if the result is within the range for the LOD.
////////////////////////////////////////////////////////////////////
INLINE bool LODNode::Switch::
in_range(float dist) const {
  return (dist >= _out && dist < _in);
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Switch::rescale
//       Access: Public
//  Description: Scales the switching distances by the indicated factor.
////////////////////////////////////////////////////////////////////
INLINE void LODNode::Switch::
rescale(float factor) {
  _in *= factor;
  _out *= factor;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Switch::write_datagram
//       Access: Public
//  Description: Writes the contents of the Switch out to the
//               datagram, presumably in preparation to writing to a
//               Bam file.
////////////////////////////////////////////////////////////////////
INLINE void LODNode::Switch::
write_datagram(Datagram &destination) const {
  destination.add_float32(_in);
  destination.add_float32(_out);
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Switch::read_datagram
//       Access: Public
//  Description: Reads the contents of the Switch from the datagram,
//               presumably in response to reading a Bam file.
////////////////////////////////////////////////////////////////////
INLINE void LODNode::Switch::
read_datagram(DatagramIterator &source) {
  _in = source.get_float32();
  _out = source.get_float32();
}



////////////////////////////////////////////////////////////////////
//     Function: LODNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODNode::
LODNode(const string &name) :
  PandaNode(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODNode::
LODNode(const LODNode &copy) :
  PandaNode(copy),
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
  cdata->_switch_vector.push_back(Switch(in, out));
  cdata->check_limits();
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
  nassertr(index >= 0 && index < (int)cdata->_switch_vector.size(), false);
  cdata->_switch_vector[index].set_range(in, out);
  cdata->check_limits();
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
  cdata->_switch_vector.clear();
  cdata->_lowest = 0;
  cdata->_highest = 0;
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
  return cdata->_switch_vector.size();
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
  nassertr(index >= 0 && index < (int)cdata->_switch_vector.size(), 0.0);
  return cdata->_switch_vector[index].get_in();
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
  nassertr(index >= 0 && index < (int)cdata->_switch_vector.size(), 0.0);
  return cdata->_switch_vector[index].get_out();
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::get_lowest_switch
//       Access: Published
//  Description: Returns the index number of the child with the lowest
//               level of detail; that is, the one that is designed to
//               be seen from the farthest away.  This is usually the
//               first child, but it is not necessarily so.
////////////////////////////////////////////////////////////////////
INLINE int LODNode::
get_lowest_switch() const {
  CDReader cdata(_cycler);
  return cdata->_lowest;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::get_highest_switch
//       Access: Published
//  Description: Returns the index number of the child with the highest
//               level of detail; that is, the one that is designed to
//               be seen from the closest to the camera.  This is
//               usually the last child, but it is not necessarily so.
////////////////////////////////////////////////////////////////////
INLINE int LODNode::
get_highest_switch() const {
  CDReader cdata(_cycler);
  return cdata->_highest;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::force_switch
//       Access: Published
//  Description: Forces the LODNode to show the indicated level
//               instead of the level that would normally be shown
//               based on the distance from the camera.
////////////////////////////////////////////////////////////////////
INLINE void LODNode::
force_switch(int index) {
  CDWriter cdata(_cycler);
  cdata->_force_switch = index;
  cdata->_got_force_switch = true;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::clear_force_switch
//       Access: Published
//  Description: Undoes the effect of a previous call to
//               force_switch() and releases the LODNode to once again
//               display the normal level.
////////////////////////////////////////////////////////////////////
INLINE void LODNode::
clear_force_switch() {
  CDWriter cdata(_cycler);
  cdata->_got_force_switch = false;
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
  cdata->_center = center;
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
  return cdata->_center;
}
