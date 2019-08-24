/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file touchTrackball.cxx
 * @author D. Lawrence
 * @date 2019-08-16
 */

#include "touchTrackball.h"

TypeHandle TouchTrackball::_type_handle;

/**
 *
 */
TouchTrackball::
TouchTrackball(const std::string &name) :
  Trackball(name)
{
  _pointer_input = define_input("pointer_events", PointerEventList::get_class_type());

  // Primary touch events trigger MouseButton::one(), so we can use this to tell
  // if we need to start tracking the gestures or not.
  watch_button(MouseButton::one());
}

GestureState TouchTrackball::
determine_gesture_state(const PointerEventList *event_list) {

  // Sure do wish we could use optionals right about now.
  PT(PointerEvent) primary;
  PT(PointerEvent) secondary;

  for (int i = 0; i < event_list->get_num_events(); i++) {
    const PointerEvent &event = event_list->get_event(i);

    if (event._sequence == 0) {
      primary = new PointerEvent(event);
      if (event._data.get_phase() == PointerPhase::began) {
        _initial_touches.first = primary;
      }
    } else {
      secondary = new PointerEvent(event);
      if (event._data.get_phase() == PointerPhase::began) {
        _initial_touches.second = secondary;
      }
    }
  }

  GestureState next_state = _gesture_state;

  // If we don't have a primary pointer, then we know we're not doing a gesture.
  if (!primary || primary->_data.get_phase() == PointerPhase::ended) {
    // Make sure it's cleared out.
    
    primary = nullptr;
    _initial_touches.first = nullptr;
    next_state = GestureState::none;

  } else if (primary->_data.get_phase() == PointerPhase::began
             && _gesture_state == GestureState::none) {
    // The primary finger just started to touch the screen.
    next_state = GestureState::one_drag;

  } else if (_gesture_state == GestureState::one_drag
             && secondary
             && secondary->_data.get_phase() != PointerPhase::ended) {
    // We're doing a single-finger drag right now, but that could change under
    // the right circumstances.
    double primary_duration = ClockObject::get_global_clock()->get_frame_time() - _initial_touches.first->_time;
    if (primary_duration < _gesture_detect_time) {
      // The primary touch started recently enough that we can potentially
      // transition to another gesture.
      double initial_diff_x = _initial_touches.first->_data.get_x() - _initial_touches.second->_data.get_x();
      double initial_diff_y = _initial_touches.first->_data.get_y() - _initial_touches.second->_data.get_y();
      double initial_dist = sqrt(initial_diff_x * initial_diff_x
                                 + initial_diff_y * initial_diff_y);

      double curr_diff_x = primary->_data.get_x() - secondary->_data.get_x();
      double curr_diff_y = primary->_data.get_y() - secondary->_data.get_y();
      double curr_dist = sqrt(curr_diff_x * curr_diff_x
                                 + curr_diff_y * curr_diff_y);

      if (curr_dist - initial_dist < _pinch_touch_thresh) {
        next_state = GestureState::two_drag;
      } else {
        next_state = GestureState::pinch;
      }
      printf("%f\n", curr_dist - initial_dist);
    }
  }

  if (!secondary || secondary->_data.get_phase() == PointerPhase::ended) {
    secondary = nullptr;
    _initial_touches.second = nullptr;
  }

  _prev_touches.first = _current_touches.first;
  _prev_touches.second = _current_touches.second;
  _current_touches.first = primary;
  _current_touches.second = secondary;

  return next_state;
}

/**
 *
 */
void TouchTrackball::
do_transmit_data(DataGraphTraverser *trav,
                 const DataNodeTransmit &input,
                 DataNodeTransmit &output) {

  if (input.has_data(_pointer_input)) {
    const PointerEventList *event_list;
    DCAST_INTO_V(event_list, input.get_data(_pointer_input).get_ptr());

    _gesture_state = determine_gesture_state(event_list);

    if (_gesture_state == GestureState::none || !_prev_touches.first || !_current_touches.first) {
      return;
    }

    if (!_rel_to.is_empty()) {
      // If we have a rel_to node, we must first adjust our rotation and
      // translation to be in those local coordinates.
      reextract();
    }

    double dx = _current_touches.first->_data.get_x() - _prev_touches.first->_data.get_x();
    double dy = _current_touches.first->_data.get_y() - _prev_touches.first->_data.get_y();

    switch (_gesture_state) {
    case GestureState::one_drag:
      // Rotation.
      _rotation *=
        LMatrix4::rotate_mat_normaxis(dx * _rotscale, LVector3::up(_cs), _cs) *
        LMatrix4::rotate_mat_normaxis(dy * _rotscale, LVector3::right(_cs), _cs);
      break;
    }

    recompute();
  }

  Trackball::do_transmit_data(trav, input, output);
}
