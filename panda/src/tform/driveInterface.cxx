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
#include <keyboardButton.h>
#include <mouseButton.h>
#include <dataGraphTraverser.h>
#include <allAttributesWrapper.h>
#include <dataRelation.h>

TypeHandle DriveInterface::_type_handle;

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
  double elapsed = ClockObject::get_global_clock()->get_frame_time() - _changed_time;

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
  _changed_time = ClockObject::get_global_clock()->get_frame_time();
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
//       Access: Published
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
  _force_roll = 0.0;
  _is_force_roll = true;

  _cs = default_coordinate_system;
  _ignore_mouse = false;

  _mods.add_button(MouseButton::one());
  _mods.add_button(MouseButton::two());
  _mods.add_button(MouseButton::three());

  _transform = new MatrixDataAttribute;
  _transform->set_value(_mat);
  _attrib.set_attribute(_transform_type, _transform);
}



////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DriveInterface::
~DriveInterface() {
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::reset
//       Access: Published
//  Description: Reinitializes the driver to the origin.
////////////////////////////////////////////////////////////////////
void DriveInterface::
reset() {
  _xyz.set(0.0, 0.0, 0.0);
  _hpr.set(0.0, 0.0, 0.0);
  _force_roll = 0.0;
  _mat = LMatrix4f::ident_mat();
  _up_arrow.clear();
  _down_arrow.clear();
  _left_arrow.clear();
  _right_arrow.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_force_roll
//       Access: Published
//  Description: Sets the force_roll state.  In this state, roll is
//               always fixed to a particular value (typically zero),
//               regardless of what it is explicitly set to via
//               set_hpr().
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_force_roll(float force_roll) {
  if (_is_force_roll) {
    // If we already had force_roll() in effect, we have to
    // recompensate for it.
    if (_force_roll != force_roll) {
      _force_roll = force_roll;
      reextract();
    }

  } else {
    _force_roll = force_roll;
    _is_force_roll = true;
  }
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

  DataGraphTraverser dgt;
  dgt.traverse_below(this, _attrib);
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

  if (_is_force_roll) {
    // We strongly discourage a roll other than _force_roll.
    decompose_matrix(_mat, scale, _hpr, _xyz, _force_roll);
  } else {
    decompose_matrix(_mat, scale, _hpr, _xyz);
  }

  if (tform_cat.is_debug()) {
    tform_cat.debug() 
      << "Reextract " << _hpr << ", " << _xyz << " from:\n";
    _mat.write(tform_cat.debug(false), 2);
  }
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
      if (!(_ignore_mouse && be._down)) {
	_mods.add_event(be);
      }

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
  double x = 0.0;
  double y = 0.0;

  const Vec3DataAttribute *xyz;
  if (get_attribute_into(xyz, data, _xyz_type)) {
    LVecBase3f p = xyz->get_value();
    x = p[0];
    y = p[1];
  }

  apply(x, y, _mods.is_any_down());
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
