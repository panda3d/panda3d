// Filename: driveInterface.cxx
// Created by:  drose (17Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "driveInterface.h"
#include "config_tform.h"

#include <compose_matrix.h>
#include <mouse.h>
#include <mouseData.h>
#include <clockObject.h>
#include <modifierButtons.h>
#include <buttonEventDataTransition.h>
#include <buttonEventDataAttribute.h>
#include <modifierButtonDataTransition.h>
#include <modifierButtonDataAttribute.h>
#include <keyboardButton.h>
#include <mouseButton.h>
#include <dataGraphTraversal.h>
#include <allAttributesWrapper.h>
#include <dftraverser.h>
#include <dataRelation.h>

TypeHandle DriveInterface::_type_handle;

TypeHandle DriveInterface::_mods_type;
TypeHandle DriveInterface::_xyz_type;
TypeHandle DriveInterface::_button_events_type;
TypeHandle DriveInterface::_transform_type;


DriveInterface::KeyHeld::
KeyHeld() {
  _down = false;
  _changed_time = 0.0;
  _effect = 0.0;
  _effect_at_change = 0.0;
}

float DriveInterface::KeyHeld::
get_effect(float ramp_up_time, float ramp_down_time) {
  double elapsed = ClockObject::get_global_clock()->get_time() - _changed_time;

  if (_down) {
    // We are currently holding down the key.  That means we base our
    // effect on the ramp_up_time.
    if (ramp_up_time == 0.0) {
      _effect = 1.0;

    } else {
      float change = elapsed / ramp_up_time;
      _effect = min(_effect_at_change + change, 1.0f);
    }

  } else {
    // We are *not* currently holding down the key.  That means we
    // base our effect on the ramp_down_time.
    if (ramp_down_time == 0.0) {
      _effect = 0.0;

    } else {
      float change = elapsed / ramp_down_time;
      _effect = max(_effect_at_change - change, 0.0f);
    }
  }

  return _effect;
}

void DriveInterface::KeyHeld::
set_key(bool down) {
  _down = down;
  _changed_time = ClockObject::get_global_clock()->get_time();
  _effect_at_change = _effect;
}

void DriveInterface::KeyHeld::
clear() {
  _down = false;
  _changed_time = 0.0;
  _effect = 0.0;
  _effect_at_change = 0.0;
}

bool DriveInterface::KeyHeld::
operator < (const DriveInterface::KeyHeld &other) const {
  if (_down != other._down) {
    // If one has the key held down and the other doesn't, the down
    // key wins.
    return _down;
  }

  // Otherwise, the most-recently changed key wins.
  return _changed_time > other._changed_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::Constructor
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
DriveInterface::
DriveInterface(const string &name) : DataNode(name) {
  _forward_speed = drive_forward_speed;
  _reverse_speed = drive_reverse_speed;
  _rotate_speed = drive_rotate_speed;
  _vertical_dead_zone = drive_vertical_dead_zone;
  _horizontal_dead_zone = drive_horizontal_dead_zone;
  _vertical_center = drive_vertical_center;
  _horizontal_center = drive_horizontal_center;

  _vertical_ramp_up_time = drive_vertical_ramp_up_time;
  _vertical_ramp_down_time = drive_vertical_ramp_down_time;
  _horizontal_ramp_up_time = drive_horizontal_ramp_up_time;
  _horizontal_ramp_down_time = drive_horizontal_ramp_down_time;

  _speed = 0.0;
  _rot_speed = 0.0;

  _xyz.set(0.0, 0.0, 0.0);
  _hpr.set(0.0, 0.0, 0.0);
  _mat = LMatrix4f::ident_mat();

  _cs = default_coordinate_system;

  _transform = new MatrixDataAttribute;
  _transform->set_value(_mat);
  _attrib.set_attribute(_transform_type, _transform);
}



////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::Destructor
//       Access: Public, Scheme
//  Description:
////////////////////////////////////////////////////////////////////
DriveInterface::
~DriveInterface() {
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_forward_speed
//       Access: Public, Scheme
//  Description: Sets the speed of full forward motion, when the mouse
//               is at the very top of the window.  This is in units
//               (e.g. feet) per second.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_forward_speed(float speed) {
  _forward_speed = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_forward_speed
//       Access: Public, Scheme
//  Description: Returns the speed of full forward motion, when the
//               mouse is at the very top of the window.  This is in
//               units (e.g. feet) per second.
////////////////////////////////////////////////////////////////////
float DriveInterface::
get_forward_speed() const {
  return _forward_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_reverse_speed
//       Access: Public, Scheme
//  Description: Sets the speed of full reverse motion, when the mouse
//               is at the very bottom of the window.  This is in
//               units (e.g. feet) per second.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_reverse_speed(float speed) {
  _reverse_speed = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_reverse_speed
//       Access: Public, Scheme
//  Description: Returns the speed of full reverse motion, when the
//               mouse is at the very bottom of the window.  This is
//               in units (e.g. feet) per second.
////////////////////////////////////////////////////////////////////
float DriveInterface::
get_reverse_speed() const {
  return _reverse_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_rotate_speed
//       Access: Public, Scheme
//  Description: Sets the maximum rate at which the user can rotate
//               left or right, when the mouse is at the very edge of
//               the window.  This is in degrees per second.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_rotate_speed(float speed) {
  _rotate_speed = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_rotate_speed
//       Access: Public, Scheme
//  Description: Returns the maximum rate at which the user can rotate
//               left or right, when the mouse is at the very edge of
//               the window.  This is in degrees per second.
////////////////////////////////////////////////////////////////////
float DriveInterface::
get_rotate_speed() const {
  return _rotate_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_vertical_dead_zone
//       Access: Public, Scheme
//  Description: Sets the size of the horizontal bar in the center of
//               the screen that represents the "dead zone" of
//               vertical motion: the region in which the mouse does
//               not report vertical motion.  This is in a fraction of
//               the window height, so 0.5 will set a dead zone as
//               large as half the screen.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_vertical_dead_zone(float speed) {
  _vertical_dead_zone = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_vertical_dead_zone
//       Access: Public, Scheme
//  Description: Returns the size of the horizontal bar in the center
//               of the screen that represents the "dead zone" of
//               vertical motion: the region in which the mouse does
//               not report vertical motion.  This is in a fraction of
//               the window height, so 0.5 will set a dead zone as
//               large as half the screen.
////////////////////////////////////////////////////////////////////
float DriveInterface::
get_vertical_dead_zone() const {
  return _vertical_dead_zone;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_horizontal_dead_zone
//       Access: Public, Scheme
//  Description: Sets the size of the vertical bar in the center of
//               the screen that represents the "dead zone" of
//               horizontal motion: the region in which the mouse does
//               not report horizontal motion.  This is in a fraction of
//               the window width, so 0.5 will set a dead zone as
//               large as half the screen.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_horizontal_dead_zone(float speed) {
  _horizontal_dead_zone = speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_horizontal_dead_zone
//       Access: Public, Scheme
//  Description: Returns the size of the vertical bar in the center
//               of the screen that represents the "dead zone" of
//               horizontal motion: the region in which the mouse does
//               not report horizontal motion.  This is in a fraction of
//               the window width, so 0.5 will set a dead zone as
//               large as half the screen.
////////////////////////////////////////////////////////////////////
float DriveInterface::
get_horizontal_dead_zone() const {
  return _horizontal_dead_zone;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_vertical_ramp_up_time
//       Access: Public, Scheme
//  Description: Sets the amount of time, in seconds, it takes between
//               the time an up or down arrow key is pressed and the
//               time it registers full forward or backward motion.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_vertical_ramp_up_time(float ramp_up_time) {
  _vertical_ramp_up_time = ramp_up_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_vertical_ramp_up_time
//       Access: Public, Scheme
//  Description: Returns the amount of time, in seconds, it takes
//               between the time an up or down arrow key is pressed
//               and the time it registers full forward or backward
//               motion.
////////////////////////////////////////////////////////////////////
float DriveInterface::
get_vertical_ramp_up_time() const {
  return _vertical_ramp_up_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_vertical_ramp_down_time
//       Access: Public, Scheme
//  Description: Sets the amount of time, in seconds, it takes between
//               the time an up or down arrow key is released and the
//               time it registers no motion.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_vertical_ramp_down_time(float ramp_down_time) {
  _vertical_ramp_down_time = ramp_down_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_vertical_ramp_down_time
//       Access: Public, Scheme
//  Description: Returns the amount of time, in seconds, it takes
//               between the time an up or down arrow key is released
//               and the time it registers no motion.
////////////////////////////////////////////////////////////////////
float DriveInterface::
get_vertical_ramp_down_time() const {
  return _vertical_ramp_down_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_horizontal_ramp_up_time
//       Access: Public, Scheme
//  Description: Sets the amount of time, in seconds, it takes between
//               the time a left or right arrow key is pressed and the
//               time it registers full rotation.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_horizontal_ramp_up_time(float ramp_up_time) {
  _horizontal_ramp_up_time = ramp_up_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_horizontal_ramp_up_time
//       Access: Public, Scheme
//  Description: Returns the amount of time, in seconds, it takes
//               between the time a left or right arrow key is pressed
//               and the time it registers full rotation.
////////////////////////////////////////////////////////////////////
float DriveInterface::
get_horizontal_ramp_up_time() const {
  return _horizontal_ramp_up_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_horizontal_ramp_down_time
//       Access: Public, Scheme
//  Description: Sets the amount of time, in seconds, it takes between
//               the time a left or right arrow key is released and the
//               time it registers no motion.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_horizontal_ramp_down_time(float ramp_down_time) {
  _horizontal_ramp_down_time = ramp_down_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_horizontal_ramp_down_time
//       Access: Public, Scheme
//  Description: Returns the amount of time, in seconds, it takes
//               between the time a left or right arrow key is released
//               and the time it registers no motion.
////////////////////////////////////////////////////////////////////
float DriveInterface::
get_horizontal_ramp_down_time() const {
  return _horizontal_ramp_down_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_speed
//       Access: Public, Scheme
//  Description: Returns the speed of the previous update in units/sec
////////////////////////////////////////////////////////////////////
float DriveInterface::
get_speed() const {
  return _speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_rot_speed
//       Access: Public, Scheme
//  Description: Returns the rot_speed of the previous update in units/sec
////////////////////////////////////////////////////////////////////
float DriveInterface::
get_rot_speed() const {
  return _rot_speed;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::reset
//       Access: Public, Scheme
//  Description: Reinitializes the driver to the origin.
////////////////////////////////////////////////////////////////////
void DriveInterface::
reset() {
  _xyz.set(0.0, 0.0, 0.0);
  _hpr.set(0.0, 0.0, 0.0);
  _mat = LMatrix4f::ident_mat();
  _up_arrow.clear();
  _down_arrow.clear();
  _left_arrow.clear();
  _right_arrow.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_pos
//       Access: Public, Scheme
//  Description: Returns the driver's position.
////////////////////////////////////////////////////////////////////
const LPoint3f &DriveInterface::
get_pos() const {
  return _xyz;
}

float DriveInterface::
get_x() const {
  return _xyz[0];
}

float DriveInterface::
get_y() const {
  return _xyz[1];
}

float DriveInterface::
get_z() const {
  return _xyz[2];
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_pos
//       Access: Public, Scheme
//  Description: Directly sets the driver's position.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_pos(const LVecBase3f &vec) {
  _xyz = vec;
  recompute();
}

void DriveInterface::
set_pos(float x, float y, float z) {
  _xyz.set(x, y, z);
  recompute();
}

void DriveInterface::
set_x(float x) {
  _xyz[0] = x;
  recompute();
}

void DriveInterface::
set_y(float y) {
  _xyz[1] = y;
  recompute();
}

void DriveInterface::
set_z(float z) {
  _xyz[2] = z;
  recompute();
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_hpr
//       Access: Public, Scheme
//  Description: Returns the driver's orientation.
////////////////////////////////////////////////////////////////////
const LVecBase3f &DriveInterface::
get_hpr() const {
  return _hpr;
}

float DriveInterface::
get_h() const {
  return _hpr[0];
}

float DriveInterface::
get_p() const {
  return _hpr[1];
}

float DriveInterface::
get_r() const {
  return _hpr[2];
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_hpr
//       Access: Public, Scheme
//  Description: Directly sets the driver's orientation.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_hpr(const LVecBase3f &hpr) {
  _hpr = hpr;
  recompute();
}

void DriveInterface::
set_hpr(float h, float p, float r) {
  _hpr.set(h, p, r);
  recompute();
}

void DriveInterface::
set_h(float h) {
  _hpr[0] = h;
  recompute();
}

void DriveInterface::
set_p(float p) {
  _hpr[1] = p;
  recompute();
}

void DriveInterface::
set_r(float r) {
  _hpr[2] = r;
  recompute();
}



////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_coordinate_system
//       Access: Public, Scheme
//  Description: Sets the coordinate system of the DriveInterface.
//               Normally, this is the default coordinate system.
//               This changes the plane the user drives around in;
//               it's normally the horizontal plane (e.g. the X-Y
//               plane in a Z-up coordinate system, or the X-Z plane
//               in a Y-up coordinate system).
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_coordinate_system(CoordinateSystem cs) {
  _cs = cs;
  recompute();
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_coordinate_system
//       Access: Public, Scheme
//  Description: Returns the coordinate system of the DriveInterface.
//               See set_coordinate_system().
////////////////////////////////////////////////////////////////////
CoordinateSystem DriveInterface::
get_coordinate_system() const {
  return _cs;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_mat
//       Access: Public, Scheme
//  Description: Stores the indicated transform in the driveInterface.
//               This is a transform in global space, regardless of
//               the rel_to node.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_mat(const LMatrix4f &mat) {
  _mat = mat;
  reextract();
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_mat
//       Access: Public, Scheme
//  Description: Fills mat with the net transformation applied by the
//               current state.
////////////////////////////////////////////////////////////////////
const LMatrix4f &DriveInterface::
get_mat() const {
  return _mat;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::force_dgraph
//       Access: Public
//  Description: This is a special kludge for DriveInterface to allow
//               us to avoid the one-frame latency after a collision.
//               It forces an immediate partial data flow for all data
//               graph nodes below this node, causing all data nodes
//               that depend on this matrix to be updated immediately.
////////////////////////////////////////////////////////////////////
void DriveInterface::
force_dgraph() {
  _transform->set_value(_mat);
  int num_children = get_num_children(DataRelation::get_class_type());
  for (int i = 0; i < num_children; i++) {
    DataGraphVisitor dgv;
    df_traverse(get_child(DataRelation::get_class_type(), i),
		dgv, AllAttributesWrapper(_attrib), 
		NullLevelState(), DataRelation::get_class_type());
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::apply
//       Access: Private
//  Description: Applies the operation indicated by the user's mouse
//               motion to the current state.  Returns the matrix
//               indicating the new state.
////////////////////////////////////////////////////////////////////
void DriveInterface::
apply(double x, double y, bool any_button) {

  // First reset the speeds
  _speed = 0.0;
  _rot_speed = 0.0;

  if (any_button) {
    // If we're holding down any of the mouse buttons, do this
    // computation based on the mouse position.

    // Determine, based on the mouse's position and the amount of time
    // elapsed since last frame, how far forward/backward we should
    // move and how much we should rotate.
    
    // First, how fast are we moving?  This is based on the mouse's
    // vertical position.

    float dead_zone_top = _vertical_center + _vertical_dead_zone;
    float dead_zone_bottom = _vertical_center - _vertical_dead_zone;

    if (y >= dead_zone_top) {
      // Motion is forward.  Compute the throttle value: the ratio of
      // the mouse pointer within the range of vertical movement.
      float throttle = 
	(min(y, 1.0) - dead_zone_top) / 
	(1.0 - dead_zone_top);
      _speed = throttle * _forward_speed;
      
    } else if (y <= dead_zone_bottom) {
      // Motion is backward.
      float throttle = 
	(max(y, -1.0) - dead_zone_bottom) / 
	(-1.0 - dead_zone_bottom);
      _speed = -throttle * _reverse_speed;
    }
    
    // Now, what's our rotational velocity?  This is based on the
    // mouse's horizontal position.

    float dead_zone_right = _horizontal_center + _horizontal_dead_zone;
    float dead_zone_left = _horizontal_center - _horizontal_dead_zone;

    if (x >= dead_zone_right) {
      // Rotation is to the right.  Compute the throttle value: the
      // ratio of the mouse pointer within the range of horizontal
      // movement.
      float throttle = 
	(min(x, 1.0) - dead_zone_right) / 
	(1.0 - dead_zone_right);
      _rot_speed = throttle * _rotate_speed;
      
    } else if (x <= dead_zone_left) {
      // Rotation is to the left.
      float throttle = 
	(max(x, -1.0) - dead_zone_left) / 
	(-1.0 - dead_zone_left);
      _rot_speed = -throttle * _rotate_speed;
    }

  } else {
    // If we're not holding down any of the mouse buttons, do this
    // computation based on the arrow keys.
    
    // Which vertical arrow key changed state more recently?
    float throttle;

    if (_up_arrow < _down_arrow) {
      throttle = _up_arrow.get_effect(_vertical_ramp_up_time,
				      _vertical_ramp_down_time);
      _speed = throttle * _forward_speed;
      _down_arrow._effect = 0.0;

    } else {
      throttle = _down_arrow.get_effect(_vertical_ramp_up_time,
					_vertical_ramp_down_time);
      _speed = -throttle * _reverse_speed;
      _up_arrow._effect = 0.0;
    }

    // Which horizontal arrow key changed state more recently?
    if (_right_arrow < _left_arrow) {
      throttle = _right_arrow.get_effect(_horizontal_ramp_up_time,
					 _horizontal_ramp_down_time);
      _rot_speed = throttle * _rotate_speed;
      _left_arrow._effect = 0.0;

    } else {
      throttle = _left_arrow.get_effect(_horizontal_ramp_up_time,
					_horizontal_ramp_down_time);
      _rot_speed = -throttle * _rotate_speed;
      _right_arrow._effect = 0.0;
    }
    _right_arrow._effect = throttle;
    _left_arrow._effect = throttle;
  }

  if (_speed == 0.0 && _rot_speed == 0.0) {
    return;
  }

  // Now how far did we move based on the amount of time elapsed?
  float distance = ClockObject::get_global_clock()->get_dt() * _speed;
  float rotation = ClockObject::get_global_clock()->get_dt() * _rot_speed;

  // Now apply the vectors.

  // rot_mat is the rotation matrix corresponding to our previous
  // heading.
  LMatrix3f rot_mat = 
    LMatrix3f::rotate_mat(_hpr[0], LVector3f::up(_cs), _cs);

  // Take a step in the direction of our previous heading.
  LVector3f step = distance * (LVector3f::forward(_cs) * rot_mat);

  // To prevent upward drift due to numerical errors, force the
  // vertical component of our step to zero (it should be pretty near
  // zero anyway).
  switch (_cs) {
  case CS_zup_right:
  case CS_zup_left:
    step[2] = 0.0;
    break;

  case CS_yup_right:
  case CS_yup_left:
    step[1] = 0.0;
    break;

  default:
    break;
  }

  _xyz += step;
  _hpr[0] -= rotation;

  recompute();
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::reextract
//       Access: Private
//  Description: Given a correctly computed _mat matrix, rederive the
//               translation and rotation elements.
////////////////////////////////////////////////////////////////////
void DriveInterface::
reextract() {
  LVecBase3f scale;
  decompose_matrix(_mat, scale, _hpr, _xyz);
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::recompute
//       Access: Private
//  Description: Rebuilds the matrix according to the stored rotation
//               and translation components.
////////////////////////////////////////////////////////////////////
void DriveInterface::
recompute() {
  compose_matrix(_mat, LVecBase3f(1.0, 1.0, 1.0), _hpr, _xyz, _cs);
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::transmit_data
//       Access: Public
//  Description: Convert mouse data into a driveInterface matrix 
////////////////////////////////////////////////////////////////////
void DriveInterface::
transmit_data(NodeAttributes &data) {
  // Look for keyboard events.
  const ButtonEventDataAttribute *b;
  if (get_attribute_into(b, data, _button_events_type)) {
    ButtonEventDataAttribute::const_iterator bi;
    for (bi = b->begin(); bi != b->end(); ++bi) {
      const ButtonEvent &be = (*bi);

      if (be._button == KeyboardButton::up()) {
	_up_arrow.set_key(be._down);
      } else if (be._button == KeyboardButton::down()) {
	_down_arrow.set_key(be._down);
      } else if (be._button == KeyboardButton::left()) {
	_left_arrow.set_key(be._down);
      } else if (be._button == KeyboardButton::right()) {
	_right_arrow.set_key(be._down);
      }
    }
  }

  // Now look for mouse activity.
  bool any_button = false;
  double x = 0.0;
  double y = 0.0;

  const Vec3DataAttribute *xyz;
  if (get_attribute_into(xyz, data, _xyz_type)) {
    LVecBase3f p = xyz->get_value();
    x = p[0];
    y = p[1];

    const ModifierButtonDataAttribute *button;
    if (get_attribute_into(button, data, _mods_type)) {
      ModifierButtons mods = button->get_mods();
      any_button = 
	mods.is_down(MouseButton::one()) ||
	mods.is_down(MouseButton::two()) ||
	mods.is_down(MouseButton::three());
    }
  }

  apply(x, y, any_button);
  _transform->set_value(_mat);

  // Now send our matrix down the pipe.
  data = _attrib;
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void DriveInterface::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "DriveInterface",
		DataNode::get_class_type());

  ModifierButtonDataTransition::init_type();
  register_data_transition(_mods_type, "ModifierButtons",
			   ModifierButtonDataTransition::get_class_type());
  Vec3DataTransition::init_type();
  register_data_transition(_xyz_type, "XYZ",
			   Vec3DataTransition::get_class_type());
  MatrixDataTransition::init_type();
  register_data_transition(_transform_type, "Transform",
			   MatrixDataTransition::get_class_type());
  ButtonEventDataTransition::init_type();
  register_data_transition(_button_events_type, "ButtonEvents",
			   ButtonEventDataTransition::get_class_type());
}
