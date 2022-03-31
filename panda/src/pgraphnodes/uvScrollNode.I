/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file uvScrollNode.I
 * @author zpavlov
 * @date 2009-06-30
 */

/**
 *
 */
INLINE UvScrollNode::
UvScrollNode(const std::string &name, PN_stdfloat u_speed, PN_stdfloat v_speed, PN_stdfloat w_speed, PN_stdfloat r_speed) :
  PandaNode(name),
  _u_speed(u_speed),
  _v_speed(v_speed),
  _w_speed(w_speed),
  _r_speed(r_speed),
  _start_time(ClockObject::get_global_clock()->get_frame_time())
{
  set_cull_callback();
}

/**
 *
 */
INLINE UvScrollNode::
UvScrollNode(const std::string &name) :
  PandaNode(name),
  _u_speed(0),
  _v_speed(0),
  _w_speed(0),
  _r_speed(0),
  _start_time(ClockObject::get_global_clock()->get_frame_time())
{
  set_cull_callback();
}


/**
 *
 */
INLINE void UvScrollNode::
set_u_speed(PN_stdfloat u_speed) {
  _u_speed = u_speed;
}

/**
 *
 */
INLINE void UvScrollNode::
set_v_speed(PN_stdfloat v_speed) {
  _v_speed = v_speed;
}

/**
 *
 */
INLINE void UvScrollNode::
set_w_speed(PN_stdfloat w_speed) {
  _w_speed = w_speed;
}

/**
 *
 */
INLINE void UvScrollNode::
set_r_speed(PN_stdfloat r_speed) {
  _r_speed = r_speed;
}

/**
 *
 */
INLINE PN_stdfloat UvScrollNode::
get_u_speed() const {
  return _u_speed;
}

/**
 *
 */
INLINE PN_stdfloat UvScrollNode::
get_v_speed() const {
  return _v_speed;
}

/**
 *
 */
INLINE PN_stdfloat UvScrollNode::
get_w_speed() const {
  return _w_speed;
}

/**
 *
 */
INLINE PN_stdfloat UvScrollNode::
get_r_speed() const {
  return _r_speed;
}

/**
 *
 */
INLINE UvScrollNode::
UvScrollNode(const UvScrollNode &copy) :
  PandaNode(copy),
  _u_speed(copy._u_speed),
  _v_speed(copy._v_speed),
  _w_speed(copy._w_speed),
  _r_speed(copy._r_speed),
  _start_time(copy._start_time)
{
}
