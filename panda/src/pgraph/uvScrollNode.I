// Filename: uvScrollNode.I
// Created by:  zpavlov (30june09)
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
//     Function: UvScrollNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UvScrollNode::
UvScrollNode(const string &name, float u_speed, float v_speed) :
  PandaNode(name),
  _u_speed(u_speed),
  _v_speed(v_speed)
{
  set_cull_callback();
  _start_time = ClockObject::get_global_clock()->get_frame_time();
}

////////////////////////////////////////////////////////////////////
//     Function: UvSctrollNode::set_u_speed
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void UvScrollNode::
set_u_speed(float u_speed) {
  _u_speed = u_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: UvSctrollNode::set_v_speed
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void UvScrollNode::
set_v_speed(float v_speed) {
  _v_speed = v_speed;
}


////////////////////////////////////////////////////////////////////
//     Function: UvSctrollNode::get_u_speed
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float UvScrollNode::
get_u_speed() const {
  return _u_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: UvSctrollNode::get_v_speed
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float UvScrollNode::
get_v_speed() const {
  return _v_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: UvScrollNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UvScrollNode::
UvScrollNode(const UvScrollNode &copy) :
  PandaNode(copy),
  _u_speed(copy._u_speed),
  _v_speed(copy._v_speed)
{
}
