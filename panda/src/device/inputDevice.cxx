/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inputDevice.cxx
 * @author rdb
 * @date 2015-12-11
 */

#include "inputDevice.h"

TypeHandle InputDevice::_type_handle;

/**
 * Defines a new InputDevice.
 */
InputDevice::
InputDevice(const string &name, DeviceClass dev_class, int flags) :
  _name(name),
  _flags(flags),
  _device_class(dev_class),
  _vendor_id(0),
  _product_id(0),
  _is_connected(true),
  _event_sequence(0),
  _enable_pointer_events(false),
  _battery_level(-1),
  _max_battery_level(-1)
{
  _button_events = new ButtonEventList;
}

/**
 *
 */
InputDevice::
InputDevice(const InputDevice &copy) {
  nassertv(false);
}

/**
 *
 */
void InputDevice::
operator = (const InputDevice &copy) {
  nassertv(false);
}

/**
 *
 */
InputDevice::
~InputDevice() {
}

/**
 * Polls the input device for new activity, to ensure it contains the latest
 * events.  This will only have any effect for some types of input devices;
 * others may be updated automatically, and this method will be a no-op.
 */
void InputDevice::
poll() {
  LightMutexHolder holder(_lock);
  do_poll();
}

/**
 * Returns true if this device has a pending button event (a mouse button or
 * keyboard button down/up), false otherwise.  If this returns true, the
 * particular event may be extracted via get_button_event().
 */
bool InputDevice::
has_button_event() const {
  LightMutexHolder holder(_lock);
  return !_button_events.is_null() && _button_events->get_num_events() > 0;
}

/**
 * Returns the list of recently-generated ButtonEvents.
 * The list is also cleared.
 */
PT(ButtonEventList) InputDevice::
get_button_events() {
  LightMutexHolder holder(_lock);
  PT(ButtonEventList) result = new ButtonEventList;
  swap(_button_events, result);
  return result;
}

/**
 * Returns true if this device has a pending pointer event (a mouse movement),
 * or false otherwise.  If this returns true, the particular event may be
 * extracted via get_pointer_event().
 */
bool InputDevice::
has_pointer_event() const {
  LightMutexHolder holder(_lock);
  return (_pointer_events != 0);
}

/**
 * Returns a PointerEventList containing all the recent pointer events.
 * Clears the list.
 */
PT(PointerEventList) InputDevice::
get_pointer_events() {
  LightMutexHolder holder(_lock);
  PT(PointerEventList) result = _pointer_events;
  _pointer_events.clear();
  return result;
}


/**
 * Called by the implementation to add a new known control.
 */
int InputDevice::
add_control(ControlAxis axis, int minimum, int maximum, bool centered) {
  AnalogState state;
  state.axis = axis;
  if (centered) {
    // Centered, eg. for sticks.
    state._scale = 2.0 / (maximum - minimum);
    state._bias = (maximum + minimum) / (double)(minimum - maximum);
  } else {
    // 0-based, eg. for triggers.
    state._scale = 1.0 / maximum;
    state._bias = 0.0;
  }
  int index = (int)_controls.size();
  _controls.push_back(state);
  return index;
}

/**
 * Called by the implementation to add a new known control.  This version
 * tries to guess whether the control is centered or not.
 */
int InputDevice::
add_control(ControlAxis axis, int minimum, int maximum) {
  bool centered = (minimum < 0)
    || axis == C_left_x
    || axis == C_left_y
    || axis == C_right_x
    || axis == C_right_y
    || axis == C_x
    || axis == C_y
    || axis == C_wheel
    || axis == C_twist
    || axis == C_rudder;
  return add_control(axis, minimum, maximum, centered);
}

/**
 * Records that a mouse movement has taken place.
 */
void InputDevice::
set_pointer(bool inwin, double x, double y, double time) {
  nassertv(_lock.debug_is_locked());
  _pointer_data._in_window = inwin;
  _pointer_data._xpos = x;
  _pointer_data._ypos = y;

  if (_enable_pointer_events) {
    int seq = _event_sequence++;
    if (_pointer_events.is_null()) {
      _pointer_events = new PointerEventList();
    }
    _pointer_events->add_event(_pointer_data._in_window,
                               _pointer_data._xpos,
                               _pointer_data._ypos,
                               seq, time);
  }
}

/**
 * Records that the mouse pointer has left the window.
 */
void InputDevice::
set_pointer_out_of_window(double time) {
  nassertv(_lock.debug_is_locked());
  _pointer_data._in_window = false;

  if (_enable_pointer_events) {
    int seq = _event_sequence++;
    if (_pointer_events.is_null()) {
      _pointer_events = new PointerEventList();
    }
    _pointer_events->add_event(_pointer_data._in_window,
                               _pointer_data._xpos,
                               _pointer_data._ypos,
                               seq, time);
  }
}

/**
 * Records that a relative mouse movement has taken place.
 */
void InputDevice::
pointer_moved(double x, double y, double time) {
  nassertv(_lock.debug_is_locked());
  _pointer_data._xpos += x;
  _pointer_data._ypos += y;

  if (device_cat.is_spam() && (x != 0 || y != 0)) {
    device_cat.spam()
      << "Pointer moved by " << x << " x " << y << "\n";
  }

  if (_enable_pointer_events) {
    int seq = _event_sequence++;
    if (_pointer_events.is_null()) {
      _pointer_events = new PointerEventList();
    }
    _pointer_events->add_event(_pointer_data._in_window,
                               _pointer_data._xpos,
                               _pointer_data._ypos,
                               x, y, seq, time);
  }
}

/**
 * Sets the state of the indicated button index, where true indicates down,
 * and false indicates up.  This may generate a ButtonEvent if the button has
 * an associated ButtonHandle.  The caller should ensure that the lock is held
 * while this call is made.
 */
void InputDevice::
button_changed(int index, bool down) {
  nassertv(_lock.debug_is_locked());
  nassertv(index >= 0);
  if (index >= (int)_buttons.size()) {
    _buttons.resize(index + 1, ButtonState());
  }

  State new_state = down ? S_down : S_up;
  if (_buttons[index].state == new_state) {
    return;
  }
  _buttons[index].state = new_state;

  ButtonHandle handle = _buttons[index].handle;

  if (device_cat.is_spam()) {
    device_cat.spam()
      << "Changed button " << index;

    if (handle != ButtonHandle::none()) {
      device_cat.spam(false) << " (" << handle << ")";
    }

    device_cat.spam(false) << " to " << (down ? "down" : "up") << "\n";
  }

  if (handle != ButtonHandle::none()) {
    _button_events->add_event(ButtonEvent(handle, down ? ButtonEvent::T_down : ButtonEvent::T_up));
  }
}

/**
 * Sets the state of the indicated analog index.  The caller should ensure that
 * the lock is held while this call is made.  This should be a number in the
 * range -1.0 to 1.0, representing the current position of the control within
 * its total range of movement.
 */
void InputDevice::
set_control_state(int index, double state) {
  nassertv(_lock.debug_is_locked());
  nassertv(index >= 0);
  if (index >= (int)_controls.size()) {
    _controls.resize(index + 1, AnalogState());
  }

  if (device_cat.is_spam() && _controls[index].state != state) {
    device_cat.spam()
      << "Changed control " << index;

    if (_controls[index].axis != C_none) {
      device_cat.spam(false) << " (" << _controls[index].axis << ")";
    }

    device_cat.spam(false) << " to " << state << "\n";
  }

  _controls[index].state = state;
  _controls[index].known = true;
}

/**
 * Like set_control_state, but instead passes a raw, unscaled value.
 */
void InputDevice::
control_changed(int index, int state) {
  nassertv(_lock.debug_is_locked());
  nassertv(index >= 0);
  if (index >= (int)_controls.size()) {
    _controls.resize(index + 1, AnalogState());
  }

  double new_state = fma((double)state, _controls[index]._scale, _controls[index]._bias);

  if (device_cat.is_spam() && _controls[index].state != new_state) {
    device_cat.spam()
      << "Changed control " << index;

    if (_controls[index].axis != C_none) {
      device_cat.spam(false) << " (" << _controls[index].axis << ")";
    }

    device_cat.spam(false) << " to " << new_state << " (raw value " << state << ")\n";
  }

  _controls[index].state = new_state;
  _controls[index].known = true;
}

/**
 * Records that a tracker movement has taken place.
 */
void InputDevice::
set_tracker(const LPoint3 &pos, const LOrientation &orient, double time) {
  nassertv(_lock.debug_is_locked());

  _tracker_data.set_pos(pos);
  _tracker_data.set_orient(orient);
  _tracker_data.set_time(time);
}

/**
 * Writes a one-line string describing the device.
 */
void InputDevice::
output(ostream &out) const {
  LightMutexHolder holder(_lock);

  out << _name << " (";

  if (!_is_connected) {
    out << "dis";
  }

  out << "connected)";

  if (_device_class != DC_unknown) {
    out << ", " << _device_class;
  }

  if (_buttons.size() > 0) {
    out << ", " << _buttons.size() << " button";
    if (_buttons.size() != 1) {
      out.put('s');
    }
  }

  if (_controls.size() > 0) {
    out << ", " << _controls.size() << " control";
    if (_controls.size() != 1) {
      out.put('s');
    }
  }

  if (_flags & IDF_has_pointer) {
    out << ", pointer";
  }
  if (_flags & IDF_has_keyboard) {
    out << ", keyboard";
  }
  if (_flags & IDF_has_tracker) {
    out << ", tracker";
  }
  if (_flags & IDF_has_vibration) {
    out << ", vibration";
  }
  if (_flags & IDF_has_battery) {
    out << ", battery";

    if (_battery_level > 0 && _max_battery_level > 0) {
      out << " [";
      short i = 0;
      for (; i < _battery_level; ++i) {
        out << '=';
      }
      for (; i < _max_battery_level; ++i) {
        out << ' ';
      }
      out << ']';
    }
  }
}

/**
 * Writes a one-line string of all of the current button states.
 */
void InputDevice::
output_buttons(ostream &out) const {
  LightMutexHolder holder(_lock);

  bool any_buttons = false;
  Buttons::const_iterator bi;
  for (bi = _buttons.begin(); bi != _buttons.end(); ++bi) {
    const ButtonState &state = (*bi);
    if (state.state != S_unknown) {
      if (any_buttons) {
        out << ", ";
      }
      any_buttons = true;
      out << (int)(bi - _buttons.begin()) << "=";
      if (state.state == S_up) {
        out << "up";
      } else {
        out << "down";
      }
    }
  }

  if (!any_buttons) {
    out << "no known buttons";
  }
}

/**
 * Writes a multi-line description of the current button states.
 */
void InputDevice::
write_buttons(ostream &out, int indent_level) const {
  bool any_buttons = false;
  Buttons::const_iterator bi;
  for (bi = _buttons.begin(); bi != _buttons.end(); ++bi) {
    const ButtonState &state = (*bi);
    if (state.state != S_unknown) {
      any_buttons = true;

      indent(out, indent_level)
        << (int)(bi - _buttons.begin()) << ". ";

      if (state.handle != ButtonHandle::none()) {
        out << "(" << state.handle << ") ";
      }

      if (state.state == S_up) {
        out << "up";
      } else {
        out << "down";
      }
      out << "\n";
    }
  }

  if (!any_buttons) {
    indent(out, indent_level)
      << "(no known buttons)\n";
  }
}

/**
 * Writes a multi-line description of the current analog control states.
 */
void InputDevice::
write_controls(ostream &out, int indent_level) const {
  LightMutexHolder holder(_lock);

  bool any_controls = false;
  Controls::const_iterator ai;
  for (ai = _controls.begin(); ai != _controls.end(); ++ai) {
    const AnalogState &state = (*ai);
    if (state.known) {
      any_controls = true;

      indent(out, indent_level)
        << (int)(ai - _controls.begin()) << ". " << state.state << "\n";
    }
  }

  if (!any_controls) {
    indent(out, indent_level)
      << "(no known analog controls)\n";
  }
}

/**
 * Sets the vibration strength.  The first argument controls a low-frequency
 * motor, if present, and the latter controls a high-frequency motor.
 * The values are within the 0-1 range.
 */
void InputDevice::
do_set_vibration(double strong, double weak) {
}

/**
 * Polls the input device for new activity, to ensure it contains the latest
 * events.  This will only have any effect for some types of input devices;
 * others may be updated automatically, and this method will be a no-op.
 */
void InputDevice::
do_poll() {
}

ostream &
operator << (ostream &out, InputDevice::DeviceClass dc) {
  switch (dc) {
  case InputDevice::DC_unknown:
    out << "unknown";
    break;

  case InputDevice::DC_virtual:
    out << "virtual";
    break;

  case InputDevice::DC_keyboard:
    out << "keyboard";
    break;

  case InputDevice::DC_mouse:
    out << "mouse";
    break;

  case InputDevice::DC_touch:
    out << "touch";
    break;

  case InputDevice::DC_gamepad:
    out << "gamepad";
    break;

  case InputDevice::DC_flight_stick:
    out << "flight_stick";
    break;

  case InputDevice::DC_steering_wheel:
    out << "steering_wheel";
    break;

  case InputDevice::DC_dance_pad:
    out << "dance_pad";
    break;

  case InputDevice::DC_hmd:
    out << "hmd";
    break;

  case InputDevice::DC_COUNT:
    break;
  }
  return out;
}

ostream &
operator << (ostream &out, InputDevice::ControlAxis axis) {
  switch (axis) {
  case InputDevice::C_none:
    out << "none";
    break;

  case InputDevice::C_left_x:
    out << "left_x";
    break;

  case InputDevice::C_left_y:
    out << "left_y";
    break;

  case InputDevice::C_left_trigger:
    out << "left_trigger";
    break;

  case InputDevice::C_right_x:
    out << "right_x";
    break;

  case InputDevice::C_right_y:
    out << "right_y";
    break;

  case InputDevice::C_right_trigger:
    out << "right_trigger";
    break;

  case InputDevice::C_x:
    out << "x";
    break;

  case InputDevice::C_y:
    out << "y";
    break;

  case InputDevice::C_trigger:
    out << "trigger";
    break;

  case InputDevice::C_throttle:
    out << "throttle";
    break;

  case InputDevice::C_twist:
    out << "twist";
    break;

  case InputDevice::C_rudder:
    out << "rudder";
    break;

  case InputDevice::C_hat_x:
    out << "hat_x";
    break;

  case InputDevice::C_hat_y:
    out << "hat_y";
    break;

  case InputDevice::C_wheel:
    out << "wheel";
    break;

  case InputDevice::C_accelerator:
    out << "accelerator";
    break;

  case InputDevice::C_brake:
    out << "brake";
    break;
  }

  return out;
}
