// Filename: driveInterface.cxx
// Created by:  drose (12Mar02)
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

#include "driveInterface.h"
#include "config_tform.h"

#include "compose_matrix.h"
#include "mouseAndKeyboard.h"
#include "mouseData.h"
#include "clockObject.h"
#include "modifierButtons.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "buttonEventList.h"
#include "dataNodeTransmit.h"
#include "dataGraphTraverser.h"

TypeHandle DriveInterface::_type_handle;
const float DriveInterface::_hpr_quantize = 0.001;

DriveInterface::KeyHeld::
KeyHeld() {
  _down = false;
  _changed_time = 0.0f;
  _effect = 0.0f;
  _effect_at_change = 0.0f;
}

float DriveInterface::KeyHeld::
get_effect(float ramp_up_time, float ramp_down_time) {
  double elapsed = ClockObject::get_global_clock()->get_frame_time() - _changed_time;
  if (_down) {
    // We are currently holding down the key.  That means we base our
    // effect on the ramp_up_time.
    if (ramp_up_time == 0.0f) {
      _effect = 1.0f;

    } else {
      float change = elapsed / ramp_up_time;
      _effect = min(_effect_at_change + change, 1.0f);
    }
  } else {
    // We are *not* currently holding down the key.  That means we
    // base our effect on the ramp_down_time.
    if (ramp_down_time == 0.0f) {
      _effect = 0.0f;

    } else {
      float change = elapsed / ramp_down_time;
      _effect = max(_effect_at_change - change, 0.0f);
    }
  }
  return _effect;
}

void DriveInterface::KeyHeld::
set_key(bool down) {
  if (_down != down) {
    _down = down;
    _changed_time = ClockObject::get_global_clock()->get_frame_time();
    _effect_at_change = _effect;
  }
}

void DriveInterface::KeyHeld::
clear() {
  _down = false;
  _changed_time = 0.0f;
  _effect = 0.0f;
  _effect_at_change = 0.0f;
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
DriveInterface(const string &name) : 
  MouseInterfaceNode(name) 
{
  _xy_input = define_input("xy", EventStoreVec2::get_class_type());
  _button_events_input = define_input("button_events", ButtonEventList::get_class_type());

  _transform_output = define_output("transform", TransformState::get_class_type());
  _velocity_output = define_output("velocity", EventStoreVec3::get_class_type());

  _transform = TransformState::make_identity();
  _velocity = new EventStoreVec3(LVector3f::zero());

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

  _speed = 0.0f;
  _rot_speed = 0.0f;

  _xyz.set(0.0f, 0.0f, 0.0f);
  _hpr.set(0.0f, 0.0f, 0.0f);

  _ignore_mouse = false;
  _force_mouse = false;
  _stop_this_frame = false;

  watch_button(MouseButton::one());
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
//  Description: Reinitializes the driver to the origin and resets any
//               knowledge about buttons being held down.
////////////////////////////////////////////////////////////////////
void DriveInterface::
reset() {
  _xyz.set(0.0f, 0.0f, 0.0f);
  _hpr.set(0.0f, 0.0f, 0.0f);
  _up_arrow.clear();
  _down_arrow.clear();
  _left_arrow.clear();
  _right_arrow.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_force_roll
//       Access: Published
//  Description: This function is no longer used and does nothing.  It
//               will be removed soon.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_force_roll(float) {
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::set_mat
//       Access: Published
//  Description: Stores the indicated transform in the DriveInterface.
////////////////////////////////////////////////////////////////////
void DriveInterface::
set_mat(const LMatrix4f &mat) {
  LVecBase3f scale, shear;
  decompose_matrix(mat, scale, shear, _hpr, _xyz);
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::get_mat
//       Access: Published
//  Description: Returns the current transform.
////////////////////////////////////////////////////////////////////
const LMatrix4f &DriveInterface::
get_mat() {
  compose_matrix(_mat, 
                 LVecBase3f(1.0f, 1.0f, 1.0f), 
                 LVecBase3f(0.0f, 0.0f, 0.0f),
                 _hpr, _xyz);
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
  _transform = TransformState::make_pos_hpr(_xyz, _hpr);
  _velocity->set_value(_vel);

  DataNodeTransmit output;
  output.reserve(get_num_outputs());
  output.set_data(_transform_output, EventParameter(_transform));
  output.set_data(_velocity_output, EventParameter(_velocity));

  DataGraphTraverser dg_trav;
  dg_trav.traverse_below(this, output);
  dg_trav.collect_leftovers();
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
  _speed = 0.0f;
  _rot_speed = 0.0f;

  if (any_button || _force_mouse) {
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
        // double 1.0, not 1.0f, is required here to satisfy min()
        (min(y, 1.0) - dead_zone_top) /
        (1.0f - dead_zone_top);
      _speed = throttle * _forward_speed;

    } else if (y <= dead_zone_bottom) {
      // Motion is backward.
      float throttle =
        (max(y, -1.0) - dead_zone_bottom) /
        (-1.0f - dead_zone_bottom);
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
        (1.0f - dead_zone_right);
      _rot_speed = throttle * _rotate_speed;

    } else if (x <= dead_zone_left) {
      // Rotation is to the left.
      float throttle =
        (max(x, -1.0) - dead_zone_left) /
        (-1.0f - dead_zone_left);
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
      _down_arrow._effect = 0.0f;

    } else {
      throttle = _down_arrow.get_effect(_vertical_ramp_up_time,
                                        _vertical_ramp_down_time);
      _speed = -throttle * _reverse_speed;
      _up_arrow._effect = 0.0f;
    }

    // Which horizontal arrow key changed state more recently?
    if (_right_arrow < _left_arrow) {
      throttle = _right_arrow.get_effect(_horizontal_ramp_up_time,
                                         _horizontal_ramp_down_time);
      _rot_speed = throttle * _rotate_speed;
      _left_arrow._effect = 0.0f;

    } else {
      throttle = _left_arrow.get_effect(_horizontal_ramp_up_time,
                                        _horizontal_ramp_down_time);
      _rot_speed = -throttle * _rotate_speed;
      _right_arrow._effect = 0.0f;
    }
    _right_arrow._effect = throttle;
    _left_arrow._effect = throttle;
  }

  if (_speed == 0.0f && _rot_speed == 0.0f) {
    _vel.set(0.0f, 0.0f, 0.0f);
    return;
  }

  // Now how far did we move based on the amount of time elapsed?
  float distance = ClockObject::get_global_clock()->get_dt() * _speed;
  float rotation = ClockObject::get_global_clock()->get_dt() * _rot_speed;
  if (_stop_this_frame) {
    distance = 0.0f;
    rotation = 0.0f;
    _stop_this_frame = false;
  }

  // Now apply the vectors.

  // rot_mat is the rotation matrix corresponding to our previous
  // heading.
  LMatrix3f rot_mat =
    LMatrix3f::rotate_mat_normaxis(_hpr[0], LVector3f::up());

  // Take a step in the direction of our previous heading.
  _vel = LVector3f::forward() * distance;
  LVector3f step = (_vel * rot_mat);

  // To prevent upward drift due to numerical errors, force the
  // vertical component of our step to zero (it should be pretty near
  // zero anyway).
  switch (get_default_coordinate_system()) {
  case CS_zup_right:
  case CS_zup_left:
    step[2] = 0.0f;
    break;

  case CS_yup_right:
  case CS_yup_left:
    step[1] = 0.0f;
    break;

  default:
    break;
  }

  _xyz += step;
  _hpr[0] -= rotation;
}

////////////////////////////////////////////////////////////////////
//     Function: DriveInterface::do_transmit_data
//       Access: Protected, Virtual
//  Description: The virtual implementation of transmit_data().  This
//               function receives an array of input parameters and
//               should generate an array of output parameters.  The
//               input parameters may be accessed with the index
//               numbers returned by the define_input() calls that
//               were made earlier (presumably in the constructor);
//               likewise, the output parameters should be set with
//               the index numbers returned by the define_output()
//               calls.
////////////////////////////////////////////////////////////////////
void DriveInterface::
do_transmit_data(const DataNodeTransmit &input, DataNodeTransmit &output) {
  // First, update our modifier buttons.
  bool required_buttons_match;
  const ButtonEventList *button_events = check_button_events(input, required_buttons_match);

  // Look for mouse activity.
  double x = 0.0f;
  double y = 0.0f;

  bool got_mouse = false;

  if (required_buttons_match && input.has_data(_xy_input)) {
    const EventStoreVec2 *xy;
    DCAST_INTO_V(xy, input.get_data(_xy_input).get_ptr());
    const LVecBase2f &p = xy->get_value();
    x = p[0];
    y = p[1];

    got_mouse = true;
  }

  // Look for keyboard events.
  if (required_buttons_match && button_events != (const ButtonEventList *)NULL) {

    int num_events = button_events->get_num_events();
    for (int i = 0; i < num_events; i++) {
      const ButtonEvent &be = button_events->get_event(i);
      if (be._type != ButtonEvent::T_keystroke) {
        bool down = (be._type == ButtonEvent::T_down);
        
        if (be._button == KeyboardButton::up()) {
          _up_arrow.set_key(down);
        } else if (be._button == KeyboardButton::down()) {
          _down_arrow.set_key(down);
        } else if (be._button == KeyboardButton::left()) {
          _left_arrow.set_key(down);
        } else if (be._button == KeyboardButton::right()) {
          _right_arrow.set_key(down);
        }
      }
    }
  }

  apply(x, y, !_ignore_mouse && is_down(MouseButton::one()));
  _transform = TransformState::make_pos_hpr(_xyz, _hpr);
  _velocity->set_value(_vel);
  output.set_data(_transform_output, EventParameter(_transform));
  output.set_data(_velocity_output, EventParameter(_velocity));
}
