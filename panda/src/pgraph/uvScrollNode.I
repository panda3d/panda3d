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
UvScrollNode(const string &name, float u_speed, float v_speed, float r_speed) :
  PandaNode(name),
  _start_time(ClockObject::get_global_clock()->get_frame_time()),
  _u_speed(u_speed),
  _v_speed(v_speed),
  _r_speed(r_speed)
{  
  set_cull_callback();
}

////////////////////////////////////////////////////////////////////
//     Function: UvScrollNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UvScrollNode::
UvScrollNode(const string &name) :
  PandaNode(name),
  _start_time(ClockObject::get_global_clock()->get_frame_time()),
  _u_speed(0),
  _v_speed(0),
  _r_speed(0)
{  
  set_cull_callback();
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
//     Function: UvSctrollNode::set_r_speed
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void UvScrollNode::
set_r_speed(float r_speed) {
  _r_speed = r_speed;
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
//     Function: UvSctrollNode::get_r_speed
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float UvScrollNode::
get_r_speed() const {
  return _r_speed;
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
  _v_speed(copy._v_speed),
  _r_speed(copy._r_speed),
  _start_time(copy._start_time)
{
}
