// Filename: linuxJoystickDevice.cxx
// Created by:  rdb (21Aug15)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "linuxJoystickDevice.h"

#ifdef PHAVE_LINUX_INPUT_H

#include "gamepadButton.h"

#include <fcntl.h>
#include <linux/joystick.h>

TypeHandle LinuxJoystickDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LinuxJoystickDevice::Constructor
//       Access: Private
//  Description: Creates a new device using the Linux joystick
//               device using the given device filename.
////////////////////////////////////////////////////////////////////
LinuxJoystickDevice::
LinuxJoystickDevice(const string &device) :
  _fd(-1),
  _device(device)
{
  LightMutexHolder holder(_lock);
  if (!open_device()) {
    device_cat.error()
      << "Could not open joystick device " << _device
      << ": " << strerror(errno) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LinuxJoystickDevice::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LinuxJoystickDevice::
~LinuxJoystickDevice() {
  if (_fd != -1) {
    close(_fd);
    _fd = -1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LinuxJoystickDevice::do_poll
//       Access: Private, Virtual
//  Description: Polls the input device for new activity, to ensure
//               it contains the latest events.  This will only have
//               any effect for some types of input devices; others
//               may be updated automatically, and this method will
//               be a no-op.
////////////////////////////////////////////////////////////////////
void LinuxJoystickDevice::
do_poll() {
  if (_fd != -1) {
    while (process_events()) {}
  } else {
    open_device();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LinuxJoystickDevice::open_device
//       Access: Private
//  Description: Opens or reopens the joystick device, and reads out
//               the button and axis mappings.
//               Assumes the lock has been held.
////////////////////////////////////////////////////////////////////
bool LinuxJoystickDevice::
open_device() {
  nassertr(_lock.debug_is_locked(), false);

  _fd = open(_device.c_str(), O_RDONLY | O_NONBLOCK);

  if (_fd == -1) {
    _is_connected = false;
    return false;
  }

  // Read the name from the device.
  char name[128];
  name[0] = 0;
  ioctl(_fd, JSIOCGNAME(sizeof(name)), name);
  _name = name;

  // Get the number of axes.
  PN_uint8 num_axes = 0, num_buttons = 0;
  ioctl(_fd, JSIOCGAXES, &num_axes);
  ioctl(_fd, JSIOCGBUTTONS, &num_buttons);

  _buttons.resize(num_buttons);
  _controls.resize(num_axes);

  if (num_buttons > 0) {
    PN_uint16 btnmap[512];
    ioctl(_fd, JSIOCGBTNMAP, btnmap);

    for (char i = 0; i < num_buttons; ++i) {
      ButtonHandle handle = ButtonHandle::none();
      switch (btnmap[i]) {
      case BTN_A:
        handle = GamepadButton::action_a();
        _device_class = DC_gamepad;
        break;

      case BTN_B:
        handle = GamepadButton::action_b();
        break;

      case BTN_C:
        handle = GamepadButton::action_c();
        break;

      case BTN_X:
        handle = GamepadButton::action_x();
        break;

      case BTN_Y:
        handle = GamepadButton::action_y();
        break;

      case BTN_Z:
        handle = GamepadButton::action_z();
        break;

      case BTN_TL:
        handle = GamepadButton::lshoulder();
        break;

      case BTN_TR:
        handle = GamepadButton::rshoulder();
        break;

      case BTN_TL2:
        handle = GamepadButton::ltrigger();
        break;

      case BTN_TR2:
        handle = GamepadButton::rtrigger();
        break;

      case BTN_SELECT:
        handle = GamepadButton::back();
        break;

      case BTN_START:
        handle = GamepadButton::start();
        break;

      case BTN_MODE:
        handle = GamepadButton::guide();
        break;

      case BTN_THUMBL:
        handle = GamepadButton::lstick();
        break;

      case BTN_THUMBR:
        handle = GamepadButton::rstick();
        break;

      case BTN_TRIGGER_HAPPY1:
        handle = GamepadButton::dpad_left();
        break;

      case BTN_TRIGGER_HAPPY2:
        handle = GamepadButton::dpad_right();
        break;

      case BTN_TRIGGER_HAPPY3:
        handle = GamepadButton::dpad_up();
        break;

      case BTN_TRIGGER_HAPPY4:
        handle = GamepadButton::dpad_down();
        break;

      default:
        handle = ButtonHandle::none();
        break;
      }
      _buttons[i]._handle = handle;
    }
  }

  if (num_axes > 0) {
    PN_uint8 axmap[512];
    ioctl(_fd, JSIOCGAXMAP, axmap);

    for (char i = 0; i < num_axes; ++i) {
      ControlAxis axis = C_none;

      switch (axmap[i]) {
      case ABS_X:
        if (_device_class == DC_gamepad) {
          axis = C_left_x;
        } else {
          axis = C_x;
        }
        break;

      case ABS_Y:
        if (_device_class == DC_gamepad) {
          axis = C_left_y;
        } else {
          axis = C_y;
        }
        break;

      case ABS_Z:
        if (_device_class == DC_gamepad) {
          axis = C_left_trigger;
        } else {
          axis = C_trigger;
        }
        break;

      case ABS_RX:
        axis = C_right_x;
        break;

      case ABS_RY:
        axis = C_right_y;
        break;

      case ABS_RZ:
        axis = C_right_trigger;
        break;

      default:
        axis = C_none;
        break;
      }
      _controls[i]._axis = axis;
    }
  }

  // Read the init events.
  _is_connected = true;
  while (process_events()) {};
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LinuxJoystickDevice::process_events
//       Access: Private
//  Description: Reads a number of events from the joystick.
////////////////////////////////////////////////////////////////////
bool LinuxJoystickDevice::
process_events() {
  // Read 8 events at a time.
  struct js_event events[8];

  int n_read = read(_fd, events, sizeof(events));
  if (n_read < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // No data available for now.

    } else if (errno == ENODEV) {
      // The device ceased to exist, so we better close it.
      close(_fd);
      _fd = -1;
      _is_connected = false;
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
      set_button_state(index, (events[i].value != 0));

    } else if (events[i].type & JS_EVENT_AXIS) {
      ControlAxis axis = _controls[index]._axis;

      if (axis == C_left_trigger || axis == C_right_trigger || axis == C_trigger) {
        // We'd like to use 0.0 to indicate the resting position.
        set_control_state(index, (events[i].value + 32767) / 65534.0);
      } else {
        set_control_state(index, events[i].value / 32767.0);
      }
    }
  }

  return true;
}

#endif
