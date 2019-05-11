/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inputDevice.I
 * @author rdb
 * @date 2015-12-11
 */

#include "config_device.h"

/**
 *
 */
INLINE InputDevice::
InputDevice() {
  _button_events = new ButtonEventList;
  _pointer_events = new PointerEventList;
}

/**
 * Returns a human-readable name for the device.  Not necessarily unique.
 */
INLINE std::string InputDevice::
get_name() const {
  LightMutexHolder holder(_lock);
  return _name;
}

/**
 * Returns a string containing the manufacturer of the device, if this
 * information is known.
 */
INLINE std::string InputDevice::
get_manufacturer() const {
  LightMutexHolder holder(_lock);
  return _manufacturer;
}

/**
 * Returns a string containing the serial number of the device, if this
 * information is known.
 */
INLINE std::string InputDevice::
get_serial_number() const {
  LightMutexHolder holder(_lock);
  return _serial_number;
}

/**
 * Returns a string containing the USB vendor ID of the device, if this
 * information is known.
 */
INLINE unsigned short InputDevice::
get_vendor_id() const {
  LightMutexHolder holder(_lock);
  return _vendor_id;
}

/**
 * Returns a string containing the USB product ID of the device, if this
 * information is known.
 */
INLINE unsigned short InputDevice::
get_product_id() const {
  LightMutexHolder holder(_lock);
  return _product_id;
}

/**
 * Returns true if the device is still connected and able to receive data,
 * false otherwise.  May return false positives.
 */
INLINE bool InputDevice::
is_connected() const {
  LightMutexHolder holder(_lock);
  return _is_connected;
}

/**
 * Returns an identification of the general type of device.  If this could not
 * be determined, returns DeviceClass.unknown.
 */
INLINE InputDevice::DeviceClass InputDevice::
get_device_class() const {
  LightMutexHolder holder(_lock);
  return _device_class;
}

/**
 * Returns true if the device supports the indicated feature.
 */
INLINE bool InputDevice::
has_feature(Feature feature) const {
  LightMutexHolder holder(_lock);
  return (_features & (1 << (unsigned int)feature)) != 0;
}

/**
 * Returns true if this is a pointing device.
 */
INLINE bool InputDevice::
has_pointer() const {
  return has_feature(Feature::pointer);
}

/**
 * Returns true if the device has a physical keyboard designed for text entry.
 */
INLINE bool InputDevice::
has_keyboard() const {
  return has_feature(Feature::keyboard);
}

/**
 * Returns true if the device features a tracker that can track position and/or
 * orientation in 3D space.
 */
INLINE bool InputDevice::
has_tracker() const {
  return has_feature(Feature::tracker);
}

/**
 * Returns true if the device has vibration motors that can be controlled by
 * calling set_vibration().
 */
INLINE bool InputDevice::
has_vibration() const {
  return has_feature(Feature::vibration);
}

/**
 * Returns true if the device may be able to provide information about its
 * battery life.
 */
INLINE bool InputDevice::
has_battery() const {
  return has_feature(Feature::battery);
}

/**
 * Returns the TrackerData associated with the input device's tracker.  This
 * only makes sense if has_tracker() also returns true.
 */
INLINE TrackerData InputDevice::
get_tracker() const {
  LightMutexHolder holder(_lock);
  return _tracker_data;
}

/**
 * Returns a rough indication of the battery level, ranging from 0 (completely
 * empty battery) to the indicated max_level value.
 */
INLINE InputDevice::BatteryData InputDevice::
get_battery() const {
  LightMutexHolder holder(_lock);
  return _battery_data;
}

/**
 * Returns the number of buttons known to the device.  This includes those
 * buttons whose state has been seen, as well as buttons that have been
 * associated with a ButtonHandle even if their state is unknown.  This number
 * may change as more buttons are discovered.
 */
INLINE size_t InputDevice::
get_num_buttons() const {
  LightMutexHolder holder(_lock);
  return _buttons.size();
}

/**
 * Associates the indicated ButtonHandle with the button of the indicated index
 * number.  When the given button index changes state, a corresponding
 * ButtonEvent will be generated with the given ButtonHandle.  Pass
 * ButtonHandle::none() to turn off any association.
 *
 * It is not necessary to call this if you simply want to query the state of
 * the various buttons by index number; this is only necessary in order to
 * generate ButtonEvents when the buttons change state.
 */
INLINE void InputDevice::
map_button(size_t index, ButtonHandle button) {
  LightMutexHolder holder(_lock);
  if (index >= _buttons.size()) {
    _buttons.resize(index + 1, ButtonState());
  }

  _buttons[index].handle = button;
}

/**
 * Returns the ButtonHandle that was previously associated with the given index
 * number by a call to map_button(), or ButtonHandle::none() if no button
 * was associated.
 */
INLINE ButtonHandle InputDevice::
get_button_map(size_t index) const {
  if (index < _buttons.size()) {
    return _buttons[index].handle;
  } else {
    return ButtonHandle::none();
  }
}

/**
 * Returns true if the indicated button (identified by its index number) is
 * currently known to be down, or false if it is up or unknown.
 */
INLINE bool InputDevice::
is_button_pressed(size_t index) const {
  if (index < _buttons.size()) {
    return (_buttons[index]._state == S_down);
  } else {
    return false;
  }
}

/**
 * Returns true if the state of the indicated button is known, or false if we
 * have never heard anything about this particular button.
 */
INLINE bool InputDevice::
is_button_known(size_t index) const {
  if (index < _buttons.size()) {
    return _buttons[index]._state != S_unknown;
  } else {
    return false;
  }
}

/**
 * Returns the ButtonState that is set at the given index, or throw an assert
 * if the index was not found in the list.
 */
INLINE InputDevice::ButtonState InputDevice::
get_button(size_t index) const {
  nassertr_always(index < _buttons.size(), ButtonState());
  return _buttons[index];
}

/**
 * Returns the first ButtonState found with the given axis, or throw an assert
 * if the button handle was not found in the list.
 */
INLINE InputDevice::ButtonState InputDevice::
find_button(ButtonHandle handle) const {
  for (size_t i = 0; i < _buttons.size(); ++i) {
    if (_buttons[i].handle == handle) {
      return _buttons[i];
    }
  }
  return ButtonState();
}

/**
 * Returns the number of analog axes known to the InputDevice.  This number
 * may change as more axes are discovered.
 */
INLINE size_t InputDevice::
get_num_axes() const {
  return _axes.size();
}

/**
 * Associates the indicated Axis with the axis of the indicated index
 * number.  Pass Axis::none to turn off any association.
 *
 * It is not necessary to call this if you simply want to query the state of
 * the various axes by index number.
 */
INLINE void InputDevice::
map_axis(size_t index, InputDevice::Axis axis) {
  LightMutexHolder holder(_lock);
  if (index >= _axes.size()) {
    _axes.resize(index + 1, AxisState());
  }

  _axes[index].axis = axis;
}

/**
 * Returns the current position of indicated analog axis (identified by its
 * index number), or 0.0 if the axis is unknown.  The normal range of a
 * single axis is -1.0 to 1.0.
 */
INLINE double InputDevice::
get_axis_value(size_t index) const {
  if (index < _axes.size()) {
    return _axes[index].value;
  } else {
    return 0.0;
  }
}

/**
 * Returns the axis state that is set at the given index, or throw an assert
 * if the index was not found in the list.
 */
INLINE InputDevice::AxisState InputDevice::
get_axis(size_t index) const {
  nassertr_always(index < _axes.size(), AxisState());
  return _axes[index];
}

/**
 * Returns the first AnalogAxis found with the given axis, or throw an assert
 * if the axis was not found in the list.
 */
INLINE InputDevice::AxisState InputDevice::
find_axis(InputDevice::Axis axis) const {
  for (size_t i = 0; i < _axes.size(); ++i) {
    if (_axes[i].axis == axis) {
      return _axes[i];
    }
  }
  return AxisState();
}

/**
 * Returns true if the state of the indicated analog axis is known, or false
 * if we have never heard anything about this particular axis.
 */
INLINE bool InputDevice::
is_axis_known(size_t index) const {
  if (index < _axes.size()) {
    return _axes[index].known;
  } else {
    return false;
  }
}

/**
 * Sets the strength of the vibration effect, if supported.  The values are
 * clamped to 0-1 range. The first value axes the low-frequency rumble
 * motor, whereas the second axes the high-frequency motor, if present.
 */
INLINE void InputDevice::
set_vibration(double strong, double weak) {
  LightMutexHolder holder(_lock);
  do_set_vibration(std::max(std::min(strong, 1.0), 0.0), std::max(std::min(weak, 1.0), 0.0));
}

/**
 * Enables the generation of mouse-movement events.
 */
INLINE void InputDevice::
enable_pointer_events() {
  LightMutexHolder holder(_lock);
  _enable_pointer_events = true;
}

/**
 * Disables the generation of mouse-movement events.
 */
INLINE void InputDevice::
disable_pointer_events() {
  LightMutexHolder holder(_lock);
  _enable_pointer_events = false;
  _pointer_events.clear();
}

/**
 * Called to indicate that the device supports the given feature.
 * Assumes the lock is held.
 */
INLINE void InputDevice::
enable_feature(Feature feature) {
  _features |= (1 << (unsigned int)feature);
}

/**
 * Called to indicate that the device has been disconnected or connected from
 * its host.
 */
INLINE void InputDevice::
set_connected(bool connected) {
  LightMutexHolder holder(_lock);
  _is_connected = connected;
}

/**
 *
 */
INLINE InputDevice::ButtonState::
ButtonState(ButtonHandle handle) :
  handle(handle) {
}

/**
 * True if the button state is currently known.
 */
ALWAYS_INLINE bool InputDevice::ButtonState::
is_known() const {
  return (_state != S_unknown);
}

/**
 * True if the button is currently known to be pressed.
 */
ALWAYS_INLINE bool InputDevice::ButtonState::
is_pressed() const {
  return (_state == S_down);
}
