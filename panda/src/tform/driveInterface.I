// Filename: driveInterface.I
// Created by:  drose (12Mar02)
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
//     Function: DriveInterface::set_forward_speed
//       Access: Published
//  Description: Sets the speed of full forward motion, when the mouse
//               is at the very top of the window.  This is in units
//               (e.g. feet) per second.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_forward_speed(PN_stdfloat speed) {
  _forward_speed = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_forward_speed
//       Access: Published
//  Description: Returns the speed of full forward motion, when the
//               mouse is at the very top of the window.  This is in
//               units (e.g. feet) per second.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DriveInterface::
get_forward_speed() const {
  return _forward_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_reverse_speed
//       Access: Published
//  Description: Sets the speed of full reverse motion, when the mouse
//               is at the very bottom of the window.  This is in
//               units (e.g. feet) per second.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_reverse_speed(PN_stdfloat speed) {
  _reverse_speed = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_reverse_speed
//       Access: Published
//  Description: Returns the speed of full reverse motion, when the
//               mouse is at the very bottom of the window.  This is
//               in units (e.g. feet) per second.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DriveInterface::
get_reverse_speed() const {
  return _reverse_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_rotate_speed
//       Access: Published
//  Description: Sets the maximum rate at which the user can rotate
//               left or right, when the mouse is at the very edge of
//               the window.  This is in degrees per second.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_rotate_speed(PN_stdfloat speed) {
  _rotate_speed = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_rotate_speed
//       Access: Published
//  Description: Returns the maximum rate at which the user can rotate
//               left or right, when the mouse is at the very edge of
//               the window.  This is in degrees per second.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DriveInterface::
get_rotate_speed() const {
  return _rotate_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_vertical_dead_zone
//       Access: Published
//  Description: Sets the size of the horizontal bar in the center of
//               the screen that represents the "dead zone" of
//               vertical motion: the region in which the mouse does
//               not report vertical motion.  This is in a fraction of
//               the window height, so 0.5 will set a dead zone as
//               large as half the screen.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_vertical_dead_zone(PN_stdfloat speed) {
  _vertical_dead_zone = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_vertical_dead_zone
//       Access: Published
//  Description: Returns the size of the horizontal bar in the center
//               of the screen that represents the "dead zone" of
//               vertical motion: the region in which the mouse does
//               not report vertical motion.  This is in a fraction of
//               the window height, so 0.5 will set a dead zone as
//               large as half the screen.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DriveInterface::
get_vertical_dead_zone() const {
  return _vertical_dead_zone;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_horizontal_dead_zone
//       Access: Published
//  Description: Sets the size of the vertical bar in the center of
//               the screen that represents the "dead zone" of
//               horizontal motion: the region in which the mouse does
//               not report horizontal motion.  This is in a fraction of
//               the window width, so 0.5 will set a dead zone as
//               large as half the screen.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_horizontal_dead_zone(PN_stdfloat speed) {
  _horizontal_dead_zone = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_horizontal_dead_zone
//       Access: Published
//  Description: Returns the size of the vertical bar in the center
//               of the screen that represents the "dead zone" of
//               horizontal motion: the region in which the mouse does
//               not report horizontal motion.  This is in a fraction of
//               the window width, so 0.5 will set a dead zone as
//               large as half the screen.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DriveInterface::
get_horizontal_dead_zone() const {
  return _horizontal_dead_zone;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_vertical_ramp_up_time
//       Access: Published
//  Description: Sets the amount of time, in seconds, it takes between
//               the time an up or down arrow key is pressed and the
//               time it registers full forward or backward motion.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_vertical_ramp_up_time(PN_stdfloat ramp_up_time) {
  _vertical_ramp_up_time = ramp_up_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_vertical_ramp_up_time
//       Access: Published
//  Description: Returns the amount of time, in seconds, it takes
//               between the time an up or down arrow key is pressed
//               and the time it registers full forward or backward
//               motion.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DriveInterface::
get_vertical_ramp_up_time() const {
  return _vertical_ramp_up_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_vertical_ramp_down_time
//       Access: Published
//  Description: Sets the amount of time, in seconds, it takes between
//               the time an up or down arrow key is released and the
//               time it registers no motion.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_vertical_ramp_down_time(PN_stdfloat ramp_down_time) {
  _vertical_ramp_down_time = ramp_down_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_vertical_ramp_down_time
//       Access: Published
//  Description: Returns the amount of time, in seconds, it takes
//               between the time an up or down arrow key is released
//               and the time it registers no motion.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DriveInterface::
get_vertical_ramp_down_time() const {
  return _vertical_ramp_down_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_horizontal_ramp_up_time
//       Access: Published
//  Description: Sets the amount of time, in seconds, it takes between
//               the time a left or right arrow key is pressed and the
//               time it registers full rotation.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_horizontal_ramp_up_time(PN_stdfloat ramp_up_time) {
  _horizontal_ramp_up_time = ramp_up_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_horizontal_ramp_up_time
//       Access: Published
//  Description: Returns the amount of time, in seconds, it takes
//               between the time a left or right arrow key is pressed
//               and the time it registers full rotation.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DriveInterface::
get_horizontal_ramp_up_time() const {
  return _horizontal_ramp_up_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_horizontal_ramp_down_time
//       Access: Published
//  Description: Sets the amount of time, in seconds, it takes between
//               the time a left or right arrow key is released and the
//               time it registers no motion.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_horizontal_ramp_down_time(PN_stdfloat ramp_down_time) {
  _horizontal_ramp_down_time = ramp_down_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_horizontal_ramp_down_time
//       Access: Published
//  Description: Returns the amount of time, in seconds, it takes
//               between the time a left or right arrow key is released
//               and the time it registers no motion.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DriveInterface::
get_horizontal_ramp_down_time() const {
  return _horizontal_ramp_down_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_speed
//       Access: Published
//  Description: Returns the speed of the previous update in units/sec
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DriveInterface::
get_speed() const {
  return _speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_rot_speed
//       Access: Published
//  Description: Returns the rot_speed of the previous update in units/sec
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DriveInterface::
get_rot_speed() const {
  return _rot_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_pos
//       Access: Published
//  Description: Returns the driver's position.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3 &DriveInterface::
get_pos() const {
  return _xyz;
}

INLINE PN_stdfloat DriveInterface::
get_x() const {
  return _xyz[0];
}

INLINE PN_stdfloat DriveInterface::
get_y() const {
  return _xyz[1];
}

INLINE PN_stdfloat DriveInterface::
get_z() const {
  return _xyz[2];
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_pos
//       Access: Published
//  Description: Directly sets the driver's position.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_pos(const LVecBase3 &vec) {
  _xyz = vec;
}

INLINE void DriveInterface::
set_pos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  _xyz.set(x, y, z);
}

INLINE void DriveInterface::
set_x(PN_stdfloat x) {
  _xyz[0] = x;
}

INLINE void DriveInterface::
set_y(PN_stdfloat y) {
  _xyz[1] = y;
}

INLINE void DriveInterface::
set_z(PN_stdfloat z) {
  _xyz[2] = z;
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_hpr
//       Access: Published
//  Description: Returns the driver's orientation.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3 &DriveInterface::
get_hpr() const {
  return _hpr;
}

INLINE PN_stdfloat DriveInterface::
get_h() const {
  return _hpr[0];
}

INLINE PN_stdfloat DriveInterface::
get_p() const {
  return _hpr[1];
}

INLINE PN_stdfloat DriveInterface::
get_r() const {
  return _hpr[2];
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_hpr
//       Access: Published
//  Description: Directly sets the driver's orientation.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_hpr(const LVecBase3 &hpr) {
  set_hpr(hpr[0], hpr[1], hpr[2]);
}

INLINE void DriveInterface::
set_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  set_h(h);
  set_p(p);
  set_r(r);
}

INLINE void DriveInterface::
set_h(PN_stdfloat h) {
  _hpr[0] = _hpr_quantize * floor(h / _hpr_quantize + 0.5f);
}

INLINE void DriveInterface::
set_p(PN_stdfloat p) {
  _hpr[1] = _hpr_quantize * floor(p / _hpr_quantize + 0.5f);
}

INLINE void DriveInterface::
set_r(PN_stdfloat r) {
  _hpr[2] = _hpr_quantize * floor(r / _hpr_quantize + 0.5f);
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_ignore_mouse
//       Access: Published
//  Description: Changes the state of the ignore_mouse flag.  If this
//               flag is true, the DriveInterface will ignore mouse
//               down button events (but still recognize mouse up
//               button events); the user will not be able to start
//               the DriveInterface going again if it is stopped, but
//               if the user is currently holding down a mouse button
//               it will not stop immediately until the user
//               eventually releases the button.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_ignore_mouse(bool ignore_mouse) {
  _ignore_mouse = ignore_mouse;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_ignore_mouse
//       Access: Published
//  Description: Returns the current setting of the ignore_mouse flag.
//               See set_ignore_mouse().
////////////////////////////////////////////////////////////////////
INLINE bool DriveInterface::
get_ignore_mouse() const {
  return _ignore_mouse;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_force_mouse
//       Access: Published
//  Description: Changes the state of the force_mouse flag.  If this
//               flag is true, the mouse button need not be held down
//               in order to drive the avatar around.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_force_mouse(bool force_mouse) {
  _force_mouse = force_mouse;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_force_mouse
//       Access: Published
//  Description: Returns the current setting of the force_mouse flag.
//               See set_force_mouse().
////////////////////////////////////////////////////////////////////
INLINE bool DriveInterface::
get_force_mouse() const {
  return _force_mouse;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_stop_this_frame
//       Access: Published
//  Description: If stop_this_frame is true, the next time the frame
//               is computed no motion will be allowed, and then the
//               flag is reset to false.  This can be used to prevent
//               too much movement when we know a long time has
//               artificially elapsed, for instance when we take a
//               screenshot, without munging the clock for everything
//               else.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_stop_this_frame(bool stop_this_frame) {
  _stop_this_frame = stop_this_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_stop_this_frame
//       Access: Published
//  Description: Returns the current setting of the stop_this_frame
//               flag.  See set_stop_this_frame().
////////////////////////////////////////////////////////////////////
INLINE bool DriveInterface::
get_stop_this_frame() const {
  return _stop_this_frame;
}
