/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linuxJoystickDevice.cxx
 * @author rdb
 * @date 2015-08-21
 */

#include "linuxJoystickDevice.h"
#include "evdevInputDevice.h"

#ifdef PHAVE_LINUX_INPUT_H

#include "gamepadButton.h"

#include <fcntl.h>
#include <linux/joystick.h>

TypeHandle LinuxJoystickDevice::_type_handle;

/**
 * Creates a new device using the Linux joystick device with the given index.
 */
LinuxJoystickDevice::
LinuxJoystickDevice(int index) :
  _fd(-1),
  _index(index),
  _dpad_x_axis(-1),
  _dpad_y_axis(-1),
  _dpad_left_button(-1),
  _dpad_up_button(-1),
  _ltrigger_button(-1),
  _rtrigger_button(-1)
{
  LightMutexHolder holder(_lock);
  if (!open_device()) {
    device_cat.error()
      << "Could not open joystick device /dev/input/js" << index
      << ": " << strerror(errno) << "\n";
  }
}

/**
 *
 */
LinuxJoystickDevice::
~LinuxJoystickDevice() {
  if (_fd != -1) {
    close(_fd);
    _fd = -1;
  }
}

/**
 * Returns true if there are pending events.
 */
bool LinuxJoystickDevice::
check_events() const {
  unsigned int avail = 0;
  ioctl(_fd, FIONREAD, &avail);
  return (avail != 0);
}

/**
 * Polls the input device for new activity, to ensure it contains the latest
 * events.  This will only have any effect for some types of input devices;
 * others may be updated automatically, and this method will be a no-op.
 */
void LinuxJoystickDevice::
do_poll() {
  if (_fd != -1 && process_events()) {
    while (process_events()) {}

    // If we got events, we are obviously connected.  Mark us so.
    if (!_is_connected) {
      _is_connected = true;
      InputDeviceManager *mgr = InputDeviceManager::get_global_ptr();
      mgr->add_device(this);
    }
  }
}

/**
 * Opens or reopens the joystick device, and reads out the button and axis
 * mappings.  Assumes the lock is already held.
 */
bool LinuxJoystickDevice::
open_device() {
  nassertr(_lock.debug_is_locked(), false);

  char path[64];
  sprintf(path, "/dev/input/js%d", _index);

  _fd = open(path, O_RDONLY | O_NONBLOCK);

  if (_fd == -1) {
    _is_connected = false;
    return false;
  }

  // Read the name from the device.  We'll later try to use sysfs to read
  // the proper product name from the device, but this is a good fallback.
  char name[128];
  name[0] = 0;
  ioctl(_fd, JSIOCGNAME(sizeof(name)), name);
  _name = name;

  bool have_analog_triggers = false;

  // Get the number of axes.
  uint8_t num_axes = 0, num_buttons = 0;
  ioctl(_fd, JSIOCGAXES, &num_axes);
  ioctl(_fd, JSIOCGBUTTONS, &num_buttons);

  _buttons.resize(num_buttons);
  _controls.resize(num_axes);

  if (num_buttons > 0) {
    uint16_t btnmap[512];
    ioctl(_fd, JSIOCGBTNMAP, btnmap);

    for (uint8_t i = 0; i < num_buttons; ++i) {
      ButtonHandle handle = EvdevInputDevice::map_button(btnmap[i]);
      if (handle == ButtonHandle::none()) {
        if (device_cat.is_debug()) {
          device_cat.debug() << "Unmapped /dev/input/js" << _index
            << " button " << (int)i << ": 0x" << hex << btnmap[i] << "\n";
        }
      } else if (handle == GamepadButton::action_a()) {
        _device_class = DC_gamepad;
      } else if (handle == GamepadButton::trigger()) {
        _device_class = DC_flight_stick;
      } else if (handle == GamepadButton::ltrigger()) {
        _ltrigger_button = i;
      } else if (handle == GamepadButton::rtrigger()) {
        _rtrigger_button = i;
      }
      _buttons[i].handle = handle;
    }
  }

  if (num_axes > 0) {
    uint8_t axmap[512];
    ioctl(_fd, JSIOCGAXMAP, axmap);

    for (uint8_t i = 0; i < num_axes; ++i) {
      ControlAxis axis = C_none;

      switch (axmap[i]) {
      case ABS_X:
        if (_device_class == DC_gamepad) {
          axis = InputDevice::C_left_x;
        } else if (_device_class == DC_flight_stick) {
          axis = InputDevice::C_roll;
        } else {
          axis = InputDevice::C_x;
        }
        break;

      case ABS_Y:
        if (_device_class == DC_gamepad) {
          axis = InputDevice::C_left_y;
        } else if (_device_class == DC_flight_stick) {
          axis = InputDevice::C_pitch;
        } else {
          axis = InputDevice::C_y;
        }
        break;

      case ABS_Z:
        if (_device_class == DC_gamepad) {
          axis = C_left_trigger;
        } else {
          //axis = C_trigger;
        }
        break;

      case ABS_RX:
        axis = C_right_x;
        break;

      case ABS_RY:
        axis = C_right_y;
        break;

      case ABS_RZ:
        if (_device_class == DC_gamepad) {
          axis = InputDevice::C_right_trigger;
        } else {
          axis = InputDevice::C_yaw;
        }
        break;

      case ABS_THROTTLE:
        axis = InputDevice::C_throttle;
        break;

      case ABS_RUDDER:
        axis = InputDevice::C_rudder;
        break;

      case ABS_WHEEL:
        axis = InputDevice::C_wheel;
        break;

      case ABS_GAS:
        axis = InputDevice::C_accelerator;
        break;

      case ABS_BRAKE:
        axis = InputDevice::C_brake;
        break;

      case ABS_HAT0X:
        if (_dpad_left_button == -1) {
          // Emulate D-Pad or hat switch.
          _dpad_x_axis = i;
          _dpad_left_button = (int)_buttons.size();
          if (_device_class == DC_gamepad) {
            _buttons.push_back(ButtonState(GamepadButton::dpad_left()));
            _buttons.push_back(ButtonState(GamepadButton::dpad_right()));
          } else {
            _buttons.push_back(ButtonState(GamepadButton::hat_left()));
            _buttons.push_back(ButtonState(GamepadButton::hat_right()));
          }
          axis = C_none;
        }
        break;

      case ABS_HAT0Y:
        if (_dpad_up_button == -1) {
          // Emulate D-Pad.
          _dpad_y_axis = i;
          _dpad_up_button = (int)_buttons.size();
          if (_device_class == DC_gamepad) {
            _buttons.push_back(ButtonState(GamepadButton::dpad_up()));
            _buttons.push_back(ButtonState(GamepadButton::dpad_down()));
          } else {
            _buttons.push_back(ButtonState(GamepadButton::hat_up()));
            _buttons.push_back(ButtonState(GamepadButton::hat_down()));
          }
          axis = C_none;
        }
        break;

      default:
        if (device_cat.is_debug()) {
          device_cat.debug() << "Unmapped /dev/input/js" << _index
            << " axis " << (int)i << ": 0x" << hex << (int)axmap[i] << "\n";
        }
        axis = C_none;
        break;
      }
      _controls[i].axis = axis;

      if (axis == C_left_trigger || axis == C_right_trigger) {
        // We'd like to use 0.0 to indicate the resting position.
        _controls[i]._scale = 1.0 / 65534.0;
        _controls[i]._bias = 0.5;
        have_analog_triggers = true;
      } else if (axis == C_left_y || axis == C_right_y || axis == C_y) {
        _controls[i]._scale = 1.0 / -32767.0;
        _controls[i]._bias = 0.0;
      } else {
        _controls[i]._scale = 1.0 / 32767.0;
        _controls[i]._bias = 0.0;
      }
    }
  }

  if (_ltrigger_button >= 0 && _rtrigger_button >= 0 && !have_analog_triggers) {
    // Emulate analog triggers.
    _ltrigger_control = (int)_controls.size();
    add_control(C_left_trigger, 0, 1, false);
    add_control(C_right_trigger, 0, 1, false);
  } else {
    _ltrigger_button = -1;
    _rtrigger_button = -1;
  }

  // Get additional information from sysfs.
  sprintf(path, "/sys/class/input/js%d/device/id/vendor", _index);
  FILE *f = fopen(path, "r");
  if (f) {
    if (fscanf(f, "%hx", &_vendor_id) < 1) {
      _vendor_id = 0;
    }
    fclose(f);
  }
  sprintf(path, "/sys/class/input/js%d/device/id/product", _index);
  f = fopen(path, "r");
  if (f) {
    if (fscanf(f, "%hx", &_product_id) < 1) {
      _product_id = 0;
    }
    fclose(f);
  }
  char buffer[256];
  sprintf(path, "/sys/class/input/js%d/device/device/../product", _index);
  f = fopen(path, "r");
  if (f) {
    if (fgets(buffer, sizeof(buffer), f) != NULL) {
      buffer[strcspn(buffer, "\r\n")] = 0;
      if (buffer[0] != 0) {
        _name.assign(buffer);
      }
    }
    fclose(f);
  }
  sprintf(path, "/sys/class/input/js%d/device/device/../manufacturer", _index);
  f = fopen(path, "r");
  if (f) {
    if (fgets(buffer, sizeof(buffer), f) != NULL) {
      buffer[strcspn(buffer, "\r\n")] = 0;
      _manufacturer.assign(buffer);
    }
    fclose(f);
  }
  sprintf(path, "/sys/class/input/js%d/device/device/../serial", _index);
  f = fopen(path, "r");
  if (f) {
    if (fgets(buffer, sizeof(buffer), f) != NULL) {
      buffer[strcspn(buffer, "\r\n")] = 0;
      _serial_number.assign(buffer);
    }
    fclose(f);
  }

  // Read the init events.
  while (process_events()) {};

  // Special case handling for the wireless Xbox receiver - the Linux
  // joystick API doesn't seem to have a way to report whether the device
  // is actually turned on.  The best we can do is check whether the axes
  // are all 0, which indicates that the driver hasn't received any data for
  // this gamepad yet (which means it hasn't been plugged in for this session)
  if (strncmp(name, "Xbox 360 Wireless Receiver", 26) == 0) {
    for (int i = 0; i < _controls.size(); ++i) {
      if (_controls[i].state != 0.0) {
        _is_connected = true;
        return true;
      }
    }
    _is_connected = false;
  } else {
    _is_connected = true;
  }

  return true;
}

/**
 * Reads a number of events from the device.  Returns true if events were
 * read, meaning this function should keep being called until it returns
 * false.
 */
bool LinuxJoystickDevice::
process_events() {
  // Read 8 events at a time.
  struct js_event events[8];

  int n_read = read(_fd, events, sizeof(events));
  if (n_read < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // No data available for now.

    } else if (errno == ENODEV) {
      // The device ceased to exist, so we better close it.  No need
      // to worry about removing it from the InputDeviceManager, as it
      // will get an inotify event sooner or later about this.
      close(_fd);
      _fd = -1;
      //_is_connected = false;
      errno = 0;

    } else {
      device_cat.error() << "read: " << strerror(errno) << "\n";
    }
    return false;
  }

  if (n_read == 0) {
    return false;
  }

  n_read /= sizeof(struct js_event);

  for (int i = 0; i < n_read; ++i) {
    int index = events[i].number;

    if (events[i].type & JS_EVENT_BUTTON) {
      if (index == _ltrigger_button) {
        control_changed(_ltrigger_control, events[i].value);
      } else if (index == _rtrigger_button) {
        control_changed(_ltrigger_control + 1, events[i].value);
      }
      button_changed(index, (events[i].value != 0));

    } else if (events[i].type & JS_EVENT_AXIS) {
      if (index == _dpad_x_axis) {
        button_changed(_dpad_left_button, events[i].value < -1000);
        button_changed(_dpad_left_button+1, events[i].value > 1000);
      } else if (index == _dpad_y_axis) {
        button_changed(_dpad_up_button, events[i].value < -1000);
        button_changed(_dpad_up_button+1, events[i].value > 1000);
      }

      control_changed(index, events[i].value);
    }
  }

  return true;
}

#endif
