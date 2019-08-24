/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseTrackball.cxx
 * @author D. Lawrence
 * @date 2019-08-17
 */

#include "mouseTrackball.h"

TypeHandle MouseTrackball::_type_handle;

// These are used internally.
#define B1_MASK 0x01
#define B2_MASK 0x02
#define B3_MASK 0x04

MouseTrackball::
MouseTrackball(const std::string &name) :
  Trackball(name)
{
  _pixel_xy_input = define_input("pixel_xy", EventStoreVec2::get_class_type());

  _last_button = 0;
  _lastx = _lasty = 0.5f;

  // We want to track the state of these buttons.
  watch_button(MouseButton::one());
  watch_button(MouseButton::two());
  watch_button(MouseButton::three());

  if (trackball_use_alt_keys) {
    // In OSX mode, we need to use the command and option key in conjunction
    // with the (one) mouse button.
    watch_button(KeyboardButton::control());
    watch_button(KeyboardButton::meta());
    watch_button(KeyboardButton::alt());
  }
}

/**
 * Applies the operation indicated by the user's mouse motion to the current
 * state.  Returns the matrix indicating the new state.
 */
void MouseTrackball::
apply(double x, double y, int button) {
  if (button && !_rel_to.is_empty()) {
    // If we have a rel_to node, we must first adjust our rotation and
    // translation to be in those local coordinates.
    reextract();
  }

  if (button == B1_MASK && _control_mode != CM_default) {
    // We have a control mode set; this may change the meaning of button 1.
    // Remap button to match the current control mode setting.
    switch (_control_mode) {
    case CM_truck:
      button = B1_MASK;
      break;

    case CM_pan:
      button = B2_MASK;
      break;

    case CM_dolly:
      button = B3_MASK;
      break;

    case CM_roll:
      button = B2_MASK | B3_MASK;
      break;

    case CM_default:
      // Not possible due to above logic.
      nassertv(false);
    }
  }

  if (button == B1_MASK) {
    // Button 1: translate in plane parallel to screen.

    _translation +=
      x * _fwdscale * LVector3::right(_cs) +
      y * _fwdscale * LVector3::down(_cs);

  } else if (button == (B2_MASK | B3_MASK)) {
    // Buttons 2 + 3: rotate about the vector perpendicular to the screen.

    _rotation *=
      LMatrix4::rotate_mat_normaxis((x - y) * _rotscale,
                            LVector3::forward(_cs), _cs);

  } else if ((button == B2_MASK) || (button == (B1_MASK | B3_MASK))) {
    // Button 2, or buttons 1 + 3: rotate about the right and up vectors.  (We
    // alternately define this as buttons 1 + 3, to support two-button mice.)

    _rotation *=
      LMatrix4::rotate_mat_normaxis(x * _rotscale, LVector3::up(_cs), _cs) *
      LMatrix4::rotate_mat_normaxis(y * _rotscale, LVector3::right(_cs), _cs);

  } else if ((button == B3_MASK) || (button == (B1_MASK | B2_MASK))) {
    // Button 3, or buttons 1 + 2: dolly in and out along the forward vector.
    // (We alternately define this as buttons 1 + 2, to support two-button
    // mice.)
    _translation -= y * _fwdscale * LVector3::forward(_cs);
  }

  if (button) {
    recompute();
  }
}

void MouseTrackball::
do_transmit_data(DataGraphTraverser *trav, const DataNodeTransmit &input,
                 DataNodeTransmit &output) {
    // First, update our modifier buttons.
  bool required_buttons_match;
  check_button_events(input, required_buttons_match);

  // Now, check for mouse motion.
  if (required_buttons_match && input.has_data(_pixel_xy_input)) {
    const EventStoreVec2 *pixel_xy;
    DCAST_INTO_V(pixel_xy, input.get_data(_pixel_xy_input).get_ptr());
    const LVecBase2 &p = pixel_xy->get_value();
    PN_stdfloat this_x = p[0];
    PN_stdfloat this_y = p[1];
    int this_button = 0;

    if (is_down(MouseButton::one())) {
      if (is_down(KeyboardButton::alt())) {
        // B1 + alt (option) = B2.
        this_button |= B2_MASK;
        if (is_down(KeyboardButton::meta()) || is_down(KeyboardButton::control())) {
          this_button |= B3_MASK;
        }

      } else if (is_down(KeyboardButton::meta()) || is_down(KeyboardButton::control())) {
        // B1 + meta (command) = B3.
        this_button |= B3_MASK;

      } else {
        // Without a special key, B1 is B1.
        this_button |= B1_MASK;
      }
    }
    if (is_down(MouseButton::two())) {
      this_button |= B2_MASK;
    }
    if (is_down(MouseButton::three())) {
      this_button |= B3_MASK;
    }

    PN_stdfloat x = this_x - _lastx;
    PN_stdfloat y = this_y - _lasty;

    if (this_button == _last_button) {
      apply(x, y, this_button);
    }

    _last_button = this_button;
    _lastx = this_x;
    _lasty = this_y;
  } else {
    _last_button = 0;
  }

  Trackball::do_transmit_data(trav, input, output);
}