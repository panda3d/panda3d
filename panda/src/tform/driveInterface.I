// Filename: driveInterface.I
// Created by:  drose (12Feb01)
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
set_forward_speed(float speed) {
  _forward_speed = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_forward_speed
//       Access: Published
//  Description: Returns the speed of full forward motion, when the
//               mouse is at the very top of the window.  This is in
//               units (e.g. feet) per second.
////////////////////////////////////////////////////////////////////
INLINE float DriveInterface::
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
set_reverse_speed(float speed) {
  _reverse_speed = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_reverse_speed
//       Access: Published
//  Description: Returns the speed of full reverse motion, when the
//               mouse is at the very bottom of the window.  This is
//               in units (e.g. feet) per second.
////////////////////////////////////////////////////////////////////
INLINE float DriveInterface::
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
set_rotate_speed(float speed) {
  _rotate_speed = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_rotate_speed
//       Access: Published
//  Description: Returns the maximum rate at which the user can rotate
//               left or right, when the mouse is at the very edge of
//               the window.  This is in degrees per second.
////////////////////////////////////////////////////////////////////
INLINE float DriveInterface::
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
set_vertical_dead_zone(float speed) {
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
INLINE float DriveInterface::
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
set_horizontal_dead_zone(float speed) {
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
INLINE float DriveInterface::
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
set_vertical_ramp_up_time(float ramp_up_time) {
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
INLINE float DriveInterface::
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
set_vertical_ramp_down_time(float ramp_down_time) {
  _vertical_ramp_down_time = ramp_down_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_vertical_ramp_down_time
//       Access: Published
//  Description: Returns the amount of time, in seconds, it takes
//               between the time an up or down arrow key is released
//               and the time it registers no motion.
////////////////////////////////////////////////////////////////////
INLINE float DriveInterface::
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
set_horizontal_ramp_up_time(float ramp_up_time) {
  _horizontal_ramp_up_time = ramp_up_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_horizontal_ramp_up_time
//       Access: Published
//  Description: Returns the amount of time, in seconds, it takes
//               between the time a left or right arrow key is pressed
//               and the time it registers full rotation.
////////////////////////////////////////////////////////////////////
INLINE float DriveInterface::
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
set_horizontal_ramp_down_time(float ramp_down_time) {
  _horizontal_ramp_down_time = ramp_down_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_horizontal_ramp_down_time
//       Access: Published
//  Description: Returns the amount of time, in seconds, it takes
//               between the time a left or right arrow key is released
//               and the time it registers no motion.
////////////////////////////////////////////////////////////////////
INLINE float DriveInterface::
get_horizontal_ramp_down_time() const {
  return _horizontal_ramp_down_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_speed
//       Access: Published
//  Description: Returns the speed of the previous update in units/sec
////////////////////////////////////////////////////////////////////
INLINE float DriveInterface::
get_speed() const {
  return _speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_rot_speed
//       Access: Published
//  Description: Returns the rot_speed of the previous update in units/sec
////////////////////////////////////////////////////////////////////
INLINE float DriveInterface::
get_rot_speed() const {
  return _rot_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_pos
//       Access: Published
//  Description: Returns the driver's position.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &DriveInterface::
get_pos() const {
  return _xyz;
}

INLINE float DriveInterface::
get_x() const {
  return _xyz[0];
}

INLINE float DriveInterface::
get_y() const {
  return _xyz[1];
}

INLINE float DriveInterface::
get_z() const {
  return _xyz[2];
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_pos
//       Access: Published
//  Description: Directly sets the driver's position.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_pos(const LVecBase3f &vec) {
  _xyz = vec;
  recompute();
}

INLINE void DriveInterface::
set_pos(float x, float y, float z) {
  _xyz.set(x, y, z);
  recompute();
}

INLINE void DriveInterface::
set_x(float x) {
  _xyz[0] = x;
  recompute();
}

INLINE void DriveInterface::
set_y(float y) {
  _xyz[1] = y;
  recompute();
}

INLINE void DriveInterface::
set_z(float z) {
  _xyz[2] = z;
  recompute();
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_hpr
//       Access: Published
//  Description: Returns the driver's orientation.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &DriveInterface::
get_hpr() const {
  return _hpr;
}

INLINE float DriveInterface::
get_h() const {
  return _hpr[0];
}

INLINE float DriveInterface::
get_p() const {
  return _hpr[1];
}

INLINE float DriveInterface::
get_r() const {
  return _hpr[2];
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_hpr
//       Access: Published
//  Description: Directly sets the driver's orientation.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_hpr(const LVecBase3f &hpr) {
  _hpr = hpr;
  recompute();
}

INLINE void DriveInterface::
set_hpr(float h, float p, float r) {
  _hpr.set(h, p, r);
  recompute();

  if (_is_force_roll && r != _force_roll) {
    reextract();
  }
}

INLINE void DriveInterface::
set_h(float h) {
  _hpr[0] = h;
  recompute();
}

INLINE void DriveInterface::
set_p(float p) {
  _hpr[1] = p;
  recompute();
}

INLINE void DriveInterface::
set_r(float r) {
  _hpr[2] = r;
  recompute();

  if (_is_force_roll && r != _force_roll) {
    reextract();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::is_force_roll
//       Access: Published
//  Description: Returns true if the force_roll state is in effect,
//               e.g. because of a previous call to set_force_roll().
//               In this state, the roll cannot be set to any value
//               other than what the force_roll value indicates.
////////////////////////////////////////////////////////////////////
INLINE bool DriveInterface::
is_force_roll() const {
  return _is_force_roll;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::clear_force_roll
//       Access: Published
//  Description: Disables the force_roll state.  See set_force_roll().
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
clear_force_roll() {
  _is_force_roll = false;
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_coordinate_system
//       Access: Published
//  Description: Sets the coordinate system of the DriveInterface.
//               Normally, this is the default coordinate system.
//               This changes the plane the user drives around in;
//               it's normally the horizontal plane (e.g. the X-Y
//               plane in a Z-up coordinate system, or the X-Z plane
//               in a Y-up coordinate system).
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_coordinate_system(CoordinateSystem cs) {
  _cs = cs;
  recompute();
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_coordinate_system
//       Access: Published
//  Description: Returns the coordinate system of the DriveInterface.
//               See set_coordinate_system().
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem DriveInterface::
get_coordinate_system() const {
  return _cs;
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
//     Function: DriveInterface::set_mat
//       Access: Published
//  Description: Stores the indicated transform in the driveInterface.
//               This is a transform in global space, regardless of
//               the rel_to node.
////////////////////////////////////////////////////////////////////
INLINE void DriveInterface::
set_mat(const LMatrix4f &mat) {
  _mat = mat;
  reextract();
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_mat
//       Access: Published
//  Description: Fills mat with the net transformation applied by the
//               current state.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &DriveInterface::
get_mat() const {
  return _mat;
}
