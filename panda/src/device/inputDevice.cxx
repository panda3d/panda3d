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

#if defined(_MSC_VER) && _MSC_VER < 1700
#define fma(a, b, c) ((a) * (b) + (c))
#endif

TypeHandle InputDevice::_type_handle;

/**
 * Defines a new InputDevice.
 */
InputDevice::
InputDevice(const std::string &name, DeviceClass dev_class) :
  _name(name),
  _device_class(dev_class),
  _is_connected(true)
{
  _button_events = new ButtonEventList;
  _pointer_events = new PointerEventList;
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
  return _pointer_events != nullptr && !_pointer_events->empty();
}

/**
 * Returns a PointerEventList containing all the recent pointer events.
 * Clears the list.
 */
PT(PointerEventList) InputDevice::
get_pointer_events() {
  LightMutexHolder holder(_lock);
  PT(PointerEventList) result = new PointerEventList;
  swap(_pointer_events, result);
  return result;
}

/**
 * Called by the implementation to add a new known button.
 */
int InputDevice::
add_button(ButtonHandle button) {
  int index = (int)_buttons.size();
  _buttons.push_back(ButtonState(button));
  return index;
}

/**
 * Called by the implementation to add a new known axis.
 */
int InputDevice::
add_axis(Axis axis, int minimum, int maximum, bool centered) {
  AxisState state;
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
  int index = (int)_axes.size();
  _axes.push_back(state);
  return index;
}

/**
 * Called by the implementation to add a new known axis.  This version tries
 * to guess whether the axis is centered or not.
 */
int InputDevice::
add_axis(Axis axis, int minimum, int maximum) {
  bool centered = (minimum < 0)
    || axis == Axis::x
    || axis == Axis::y
    || axis == Axis::z
    || axis == Axis::yaw
    || axis == Axis::pitch
    || axis == Axis::roll
    || axis == Axis::left_x
    || axis == Axis::left_y
    || axis == Axis::right_x
    || axis == Axis::right_y
    || axis == Axis::wheel
    || axis == Axis::rudder;
  return add_axis(axis, minimum, maximum, centered);
}

/**
 * Records that a new pointer was found.
 */
int InputDevice::
add_pointer(PointerType type, int id, bool primary) {
  //nassertr(_lock.debug_is_locked(), -1);

  PointerData data;
  data._id = id;
  data._type = type;

  int index = (int)_pointers.size();
  if (_num_pointers == _pointers.size()) {
    _pointers.push_back(data);
  } else {
    _pointers[index] = data;
  }
  ++_num_pointers;

  return index;
}

/**
 * Removes a previously added pointer.  If the current pressure is not zero,
 * it will generate an event doing so.
 */
void InputDevice::
remove_pointer(int id) {
  nassertv(_lock.debug_is_locked());

  size_t i;
  for (i = 0; i < _pointers.size(); ++i) {
    if (_pointers[i]._id == id) {
      break;
    }
  }

  if (i < _pointers.size()) {
    if (_pointers[i]._pressure != 0.0) {
      _pointers[i]._pressure = 0.0;

      if (_enable_pointer_events) {
        int seq = _event_sequence++;
        double time = ClockObject::get_global_clock()->get_frame_time();
        _pointer_events->add_event(_pointers[i], seq, time);
      }
    }

    // Replace it with the last one.
    if (i != _pointers.size() - 1) {
      _pointers[i] = _pointers.back();
    }
    --_num_pointers;
  }
}

/**
 * Records that pointer data for a pointer has changed.  This can also be used
 * to add a new pointer.
 */
void InputDevice::
update_pointer(PointerData data, double time) {
  nassertv(_lock.debug_is_locked());

  PointerData *ptr = nullptr;
  for (size_t i = 0; i < _pointers.size(); ++i) {
    if (_pointers[i]._id == data._id) {
      ptr = &_pointers[i];
      *ptr = data;
      break;
    }
  }
  if (ptr == nullptr) {
    _pointers.push_back(data);
    ptr = &_pointers.back();
  }

  if (_enable_pointer_events) {
    int seq = _event_sequence++;
    _pointer_events->add_event(*ptr, seq, time);
  }
}

/**
 * Records that a relative pointer movement has taken place.
 */
void InputDevice::
pointer_moved(int id, double x, double y, double time) {
  nassertv(_lock.debug_is_locked());

  PointerData *ptr = nullptr;
  for (size_t i = 0; i < _pointers.size(); ++i) {
    if (_pointers[i]._id == id) {
      ptr = &_pointers[i];
      _pointers[i]._xpos = x;
      _pointers[i]._ypos = y;
      break;
    }
  }
  nassertv_always(ptr != nullptr);

  if (device_cat.is_spam() && (x != 0 || y != 0)) {
    device_cat.spam()
      << "Pointer " << id << " moved by " << x << " x " << y << "\n";
  }

  if (_enable_pointer_events) {
    int seq = _event_sequence++;
    _pointer_events->add_event(ptr->_in_window,
                               ptr->_xpos,
                               ptr->_ypos,
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
  if (_buttons[index]._state == new_state) {
    return;
  }
  _buttons[index]._state = new_state;

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
 * range -1.0 to 1.0, representing the current position of the axis within its
 * total range of movement.
 */
void InputDevice::
set_axis_value(int index, double value) {
  LightMutexHolder holder(_lock);

  nassertv(index >= 0);
  if ((size_t)index >= _axes.size()) {
    _axes.resize((size_t)index + 1u, AxisState());
  }

  if (device_cat.is_spam() && _axes[index].value != value) {
    device_cat.spam()
      << "Changed axis " << index;

    if (_axes[index].axis != Axis::none) {
      device_cat.spam(false) << " (" << _axes[index].axis << ")";
    }

    device_cat.spam(false) << " to " << value << "\n";
  }

  _axes[index].value = value;
  _axes[index].known = true;
}

/**
 * Called by the implementation during do_poll to indicate that the indicated
 * axis has received a new raw value.  Assumes the lock is held.
 */
void InputDevice::
axis_changed(int index, int state) {
  nassertv(_lock.debug_is_locked());
  nassertv(index >= 0);
  if ((size_t)index >= _axes.size()) {
    _axes.resize((size_t)index + 1u, AxisState());
  }

  double value = fma((double)state, _axes[index]._scale, _axes[index]._bias);

  if (device_cat.is_spam() && !IS_NEARLY_EQUAL(_axes[index].value, value)) {
    device_cat.spam()
      << "Changed axis " << index;

    if (_axes[index].axis != Axis::none) {
      device_cat.spam(false) << " (" << _axes[index].axis << ")";
    }

    device_cat.spam(false) << " to " << value << " (raw value " << state << ")\n";
  }

  _axes[index].value = value;
  _axes[index].known = true;
}

/**
 * Records that a tracker movement has taken place.
 */
void InputDevice::
tracker_changed(const LPoint3 &pos, const LOrientation &orient, double time) {
  nassertv(_lock.debug_is_locked());

  _tracker_data.set_pos(pos);
  _tracker_data.set_orient(orient);
  _tracker_data.set_time(time);
}

/**
 * Writes a one-line string describing the device.
 */
void InputDevice::
output(std::ostream &out) const {
  LightMutexHolder holder(_lock);

  out << _name << " (";

  if (!_is_connected) {
    out << "dis";
  }

  out << "connected)";

  if (_device_class != DeviceClass::unknown) {
    out << ", " << _device_class;
  }

  if (_buttons.size() > 0) {
    out << ", " << _buttons.size() << " button";
    if (_buttons.size() != 1) {
      out.put('s');
    }
  }

  if (_axes.size() > 0) {
    out << ", " << _axes.size() << " ax"
        << (_axes.size() != 1 ? 'e' : 'i') << 's';
  }

  if (_features & (1 << (unsigned int)Feature::pointer)) {
    out << ", pointer";
  }
  if (_features & (1 << (unsigned int)Feature::keyboard)) {
    out << ", keyboard";
  }
  if (_features & (1 << (unsigned int)Feature::tracker)) {
    out << ", tracker";
  }
  if (_features & (1 << (unsigned int)Feature::vibration)) {
    out << ", vibration";
  }
  if (_features & (1 << (unsigned int)Feature::battery)) {
    out << ", battery";

    if (_battery_data.level > 0 && _battery_data.max_level > 0) {
      out << " [";
      short i = 0;
      for (; i < _battery_data.level; ++i) {
        out << '=';
      }
      for (; i < _battery_data.max_level; ++i) {
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
output_buttons(std::ostream &out) const {
  LightMutexHolder holder(_lock);

  bool any_buttons = false;
  Buttons::const_iterator bi;
  for (bi = _buttons.begin(); bi != _buttons.end(); ++bi) {
    const ButtonState &state = (*bi);
    if (state.is_known()) {
      if (any_buttons) {
        out << ", ";
      }
      any_buttons = true;
      out << (int)(bi - _buttons.begin()) << "=";
      if (state._state == S_up) {
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
write_buttons(std::ostream &out, int indent_level) const {
  bool any_buttons = false;
  Buttons::const_iterator bi;
  for (bi = _buttons.begin(); bi != _buttons.end(); ++bi) {
    const ButtonState &state = (*bi);
    if (state.is_known()) {
      any_buttons = true;

      indent(out, indent_level)
        << (int)(bi - _buttons.begin()) << ". ";

      if (state.handle != ButtonHandle::none()) {
        out << "(" << state.handle << ") ";
      }

      if (state._state == S_up) {
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
 * Writes a multi-line description of the current analog axis states.
 */
void InputDevice::
write_axes(std::ostream &out, int indent_level) const {
  LightMutexHolder holder(_lock);

  bool any_axis = false;
  Axes::const_iterator ai;
  for (ai = _axes.begin(); ai != _axes.end(); ++ai) {
    const AxisState &state = (*ai);
    if (state.known) {
      any_axis = true;

      indent(out, indent_level)
        << (int)(ai - _axes.begin()) << ". " << state.value << "\n";
    }
  }

  if (!any_axis) {
    indent(out, indent_level)
      << "(no known analog axes)\n";
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

/**
 * Returns a string describing the given device class enumerant.
 */
std::string InputDevice::
format_device_class(DeviceClass dc) {
  switch (dc) {
  case InputDevice::DeviceClass::unknown:
    return "unknown";

  case InputDevice::DeviceClass::virtual_device:
    return "virtual_device";

  case InputDevice::DeviceClass::keyboard:
    return "keyboard";

  case InputDevice::DeviceClass::mouse:
    return "mouse";

  case InputDevice::DeviceClass::touch:
    return "touch";

  case InputDevice::DeviceClass::gamepad:
    return "gamepad";

  case InputDevice::DeviceClass::flight_stick:
    return "flight_stick";

  case InputDevice::DeviceClass::steering_wheel:
    return "steering_wheel";

  case InputDevice::DeviceClass::dance_pad:
    return "dance_pad";

  case InputDevice::DeviceClass::hmd:
    return "hmd";

  case InputDevice::DeviceClass::spatial_mouse:
    return "spatial_mouse";
  }
  return "**invalid**";
}

/**
 * Returns a string describing the given axis enumerant.
 */
std::string InputDevice::
format_axis(Axis axis) {
  switch (axis) {
  case InputDevice::Axis::none:
    return "none";

  case InputDevice::Axis::x:
    return "x";

  case InputDevice::Axis::y:
    return "y";

  case InputDevice::Axis::z:
    return "z";

  case InputDevice::Axis::yaw:
    return "yaw";

  case InputDevice::Axis::pitch:
    return "pitch";

  case InputDevice::Axis::roll:
    return "roll";

  case InputDevice::Axis::left_x:
    return "left_x";

  case InputDevice::Axis::left_y:
    return "left_y";

  case InputDevice::Axis::left_trigger:
    return "left_trigger";

  case InputDevice::Axis::right_x:
    return "right_x";

  case InputDevice::Axis::right_y:
    return "right_y";

  case InputDevice::Axis::right_trigger:
    return "right_trigger";

  //case InputDevice::Axis::trigger:
  //  return "trigger";

  case InputDevice::Axis::throttle:
    return "throttle";

  case InputDevice::Axis::rudder:
    return "rudder";

  case InputDevice::Axis::wheel:
    return "wheel";

  case InputDevice::Axis::accelerator:
    return "accelerator";

  case InputDevice::Axis::brake:
    return "brake";
  }
  return "**invalid**";
}

std::ostream &
operator << (std::ostream &out, InputDevice::DeviceClass dc) {
  out << InputDevice::format_device_class(dc);
  return out;
}

std::ostream &
operator << (std::ostream &out, InputDevice::Axis axis) {
  out << InputDevice::format_axis(axis);
  return out;
}
