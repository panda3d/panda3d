// Filename: sequenceNode.I
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
//     Function: SequenceNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SequenceNode::CData::
CData() {
  _cycle_rate = 0.0f;
  _frame_offset = 0.0f;
  _start_time = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SequenceNode::CData::
CData(const SequenceNode::CData &copy) :
  _cycle_rate(copy._cycle_rate),
  _frame_offset(copy._frame_offset),
  _start_time(copy._start_time)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SequenceNode::
SequenceNode(float cycle_rate, const string &name) :
  SelectiveChildNode(name)
{
  CDWriter cdata(_cycler);
  cdata->_cycle_rate = cycle_rate;
  cdata->_frame_offset = 0.0f;

  float now = ClockObject::get_global_clock()->get_frame_time();
  cdata->_start_time = now;
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::set_cycle_rate
//       Access: Published
//  Description: Sets the rate of cycling for the children of the
//               SequenceNode, in cycles per second.
////////////////////////////////////////////////////////////////////
INLINE void SequenceNode::
set_cycle_rate(float cycle_rate) {
  // Do some fussing so we keep the same frame visible while we
  // change this.
  CDWriter cdata(_cycler);
  float now = ClockObject::get_global_clock()->get_frame_time();
  cdata->_frame_offset = calc_frame(now);
  cdata->_start_time = now;
  cdata->_cycle_rate = cycle_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::get_cycle_rate
//       Access: Published
//  Description: Returns the rate of cycling for the children of the
//               SequenceNode, in cycles per second.
////////////////////////////////////////////////////////////////////
INLINE float SequenceNode::
get_cycle_rate() const {
  CDReader cdata(_cycler);
  return cdata->_cycle_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::set_visible_child
//       Access: Published
//  Description: Sets the particular child that this SequenceNode will
//               display this frame.  Future frames will proceed from
//               here.
////////////////////////////////////////////////////////////////////
INLINE void SequenceNode::
set_visible_child(int index) {
  int num_children = get_num_children();
  if (num_children != 0) {
    CDWriter cdata(_cycler);
    float now = ClockObject::get_global_clock()->get_frame_time();
    cdata->_frame_offset = (index - (now - cdata->_start_time) * cdata->_cycle_rate);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::calc_frame
//       Access: Private
//  Description: Returns the floating-point frame number at the
//               indicated time.
////////////////////////////////////////////////////////////////////
INLINE float SequenceNode::
calc_frame(float now) const {
  CDReader cdata(_cycler);
  return (now - cdata->_start_time) * cdata->_cycle_rate + cdata->_frame_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::calc_frame
//       Access: Private
//  Description: Returns the floating-point frame number at the
//               current time.
////////////////////////////////////////////////////////////////////
INLINE float SequenceNode::
calc_frame() const {
  return calc_frame(ClockObject::get_global_clock()->get_frame_time());
}
