// Filename: evdevInputDevice.cxx
// Created by:  rdb (24Aug15)
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

#include "evdevInputDevice.h"

#ifdef PHAVE_LINUX_INPUT_H

#include "gamepadButton.h"
#include "keyboardButton.h"
#include "mouseButton.h"

#include <fcntl.h>
#include <linux/input.h>

#define test_bit(bit, array) ((array)[(bit)/8] & (1<<((bit)&7)))

static InputDevice::ControlAxis axis_map[] = {
  InputDevice::C_left_x, InputDevice::C_left_y, InputDevice::C_left_trigger,
  InputDevice::C_right_x, InputDevice::C_right_y, InputDevice::C_right_trigger
};

TypeHandle EvdevInputDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EvdevInputDevice::Constructor
//       Access: Published
//  Description: Creates a new device using the Linux joystick
//               device using the given file descriptor.  It will
//               be closed when this object destructs.
////////////////////////////////////////////////////////////////////
EvdevInputDevice::
EvdevInputDevice(int fd) :
  _fd(fd) {
  init_device();
}

////////////////////////////////////////////////////////////////////
//     Function: EvdevInputDevice::Constructor
//       Access: Published
//  Description: Creates a new device using the Linux joystick
//               device using the given device filename.
////////////////////////////////////////////////////////////////////
EvdevInputDevice::
EvdevInputDevice(const string &fn) {
  _fd = open(fn.c_str(), O_RDONLY | O_NONBLOCK);

  if (_fd >= 0) {
    init_device();
  } else {
    _is_connected = false;
    device_cat.error()
      << "Opening raw input device: " << strerror(errno) << " " << fn << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EvdevInputDevice::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EvdevInputDevice::
~EvdevInputDevice() {
  if (_fd != -1) {
    close(_fd);
    _fd = -1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EvdevInputDevice::do_poll
//       Access: Private, Virtual
//  Description: Polls the input device for new activity, to ensure
//               it contains the latest events.  This will only have
//               any effect for some types of input devices; others
//               may be updated automatically, and this method will
//               be a no-op.
////////////////////////////////////////////////////////////////////
void EvdevInputDevice::
do_poll() {
  if (_fd != -1) {
    while (process_events()) {}
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EvdevInputDevice::init_device
//       Access: Private
//  Description: Reads basic properties from the device.
////////////////////////////////////////////////////////////////////
bool EvdevInputDevice::
init_device() {
  nassertr(_fd >= 0, false);

  uint8_t evtypes[(EV_CNT + 7) >> 3];
  memset(evtypes, 0, sizeof(evtypes));
  char name[256];
  char uniq[256];
  if (ioctl(_fd, EVIOCGNAME(sizeof(name)), name) < 0 ||
      ioctl(_fd, EVIOCGPHYS(sizeof(uniq)), uniq) < 0 ||
      ioctl(_fd, EVIOCGBIT(0, sizeof(evtypes)), evtypes) < 0) {
    close(_fd);
    _fd = -1;
    _is_connected = false;
    device_cat.error() << "Opening raw input device: ioctl failed\n";
    return false;
  }

  for (char *p=name; *p; p++) {
    if (((*p<'a')||(*p>'z')) && ((*p<'A')||(*p>'Z')) && ((*p<'0')||(*p>'9'))) {
      *p = '_';
    }
  }
  for (char *p=uniq; *p; p++) {
    if (((*p<'a')||(*p>'z')) && ((*p<'A')||(*p>'Z')) && ((*p<'0')||(*p>'9'))) {
      *p = '_';
    }
  }

  _name = ((string)name) + "." + uniq;

  if (test_bit(EV_ABS, evtypes)) {
    // Check which axes are on the device.
    uint8_t axes[(ABS_CNT + 7) >> 3];
    memset(axes, 0, sizeof(axes));

    int num_bits = ioctl(_fd, EVIOCGBIT(EV_ABS, sizeof(axes)), axes) << 3;
    for (int i = 0; i < num_bits; ++i) {
      if (test_bit(i, axes)) {
        if (i >= 0 && i < 6) {
          set_control_map(i, axis_map[i]);
        }
      }
    }
  }

  if (test_bit(EV_KEY, evtypes)) {
    // Check which buttons are on the device.
    uint8_t keys[(KEY_CNT + 7) >> 3];
    memset(keys, 0, sizeof(keys));

    int num_bits = ioctl(_fd, EVIOCGBIT(EV_KEY, sizeof(keys)), keys) << 3;
    int bi = 0;
    for (int i = 0; i < num_bits; ++i) {
      if (test_bit(i, keys)) {
        set_button_map(bi++, map_button(i));
      }
    }
  }

  if (test_bit(EV_REL, evtypes)) {
    _flags |= IDF_has_pointer;
  }

  if (test_bit(EV_FF, evtypes)) {
    _flags |= IDF_has_vibration;
  }

  _is_connected = true;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EvdevInputDevice::process_events
//       Access: Private
//  Description: Reads a number of events from the device.
////////////////////////////////////////////////////////////////////
bool EvdevInputDevice::
process_events() {
  // Read 8 events at a time.
  struct input_event events[8];

  int n_read = read(_fd, events, sizeof(events));
  if (n_read < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // No data available for now.

    } else if (errno == ENODEV || errno == EINVAL) {
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

  n_read /= sizeof(struct input_event);

  int x = _pointer_data.get_x();
  int y = _pointer_data.get_y();
  bool have_pointer = false;
  double time = ClockObject::get_global_clock()->get_frame_time();
  ButtonHandle button;

  for (int i = 0; i < n_read; ++i) {
    switch (events[i].type) {
    case EV_SYN:
      break;

    case EV_REL:
      if (events[i].code == REL_X) x += events[i].value;
      if (events[i].code == REL_Y) y += events[i].value;
      have_pointer = true;
      break;

    case EV_ABS:
      set_control_state(events[i].code, events[i].value / 32767.0);
      break;

    case EV_KEY:
      button = map_button(events[i].code);
      _button_events->add_event(ButtonEvent(button, events[i].value ? ButtonEvent::T_down : ButtonEvent::T_up, time));
      break;

    default:
      //cerr << "event " << events[i].type << " - " << events[i].code << " - " << events[i].value << "\n";
      break;
    }
  }

  if (have_pointer) {
    set_pointer(true, x, y, time);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EvdevInputDevice::map_button
//       Access: Public, Static
//  Description: Maps an evdev code to a ButtonHandle.
////////////////////////////////////////////////////////////////////
ButtonHandle EvdevInputDevice::
map_button(int code) {
  if (code >= 0 && code < 0x80) {
    // See linux/input.h for the source of this mapping.
    static const ButtonHandle keyboard_map[] = {
      ButtonHandle::none(),
      KeyboardButton::escape(),
      KeyboardButton::ascii_key('1'),
      KeyboardButton::ascii_key('2'),
      KeyboardButton::ascii_key('3'),
      KeyboardButton::ascii_key('4'),
      KeyboardButton::ascii_key('5'),
      KeyboardButton::ascii_key('6'),
      KeyboardButton::ascii_key('7'),
      KeyboardButton::ascii_key('8'),
      KeyboardButton::ascii_key('9'),
      KeyboardButton::ascii_key('0'),
      KeyboardButton::ascii_key('-'),
      KeyboardButton::ascii_key('='),
      KeyboardButton::backspace(),
      KeyboardButton::tab(),
      KeyboardButton::ascii_key('q'),
      KeyboardButton::ascii_key('w'),
      KeyboardButton::ascii_key('e'),
      KeyboardButton::ascii_key('r'),
      KeyboardButton::ascii_key('t'),
      KeyboardButton::ascii_key('y'),
      KeyboardButton::ascii_key('u'),
      KeyboardButton::ascii_key('i'),
      KeyboardButton::ascii_key('o'),
      KeyboardButton::ascii_key('p'),
      KeyboardButton::ascii_key('['),
      KeyboardButton::ascii_key(']'),
      KeyboardButton::enter(),
      KeyboardButton::lcontrol(),
      KeyboardButton::ascii_key('a'),
      KeyboardButton::ascii_key('s'),
      KeyboardButton::ascii_key('d'),
      KeyboardButton::ascii_key('f'),
      KeyboardButton::ascii_key('g'),
      KeyboardButton::ascii_key('h'),
      KeyboardButton::ascii_key('j'),
      KeyboardButton::ascii_key('k'),
      KeyboardButton::ascii_key('l'),
      KeyboardButton::ascii_key(';'),
      KeyboardButton::ascii_key('\''),
      KeyboardButton::ascii_key('`'),
      KeyboardButton::lshift(),
      KeyboardButton::ascii_key('\\'),
      KeyboardButton::ascii_key('z'),
      KeyboardButton::ascii_key('x'),
      KeyboardButton::ascii_key('c'),
      KeyboardButton::ascii_key('v'),
      KeyboardButton::ascii_key('b'),
      KeyboardButton::ascii_key('n'),
      KeyboardButton::ascii_key('m'),
      KeyboardButton::ascii_key(','),
      KeyboardButton::ascii_key('.'),
      KeyboardButton::ascii_key('/'),
      KeyboardButton::rshift(),
      KeyboardButton::ascii_key('*'),
      KeyboardButton::lalt(),
      KeyboardButton::space(),
      KeyboardButton::caps_lock(),
      KeyboardButton::f1(),
      KeyboardButton::f2(),
      KeyboardButton::f3(),
      KeyboardButton::f4(),
      KeyboardButton::f5(),
      KeyboardButton::f6(),
      KeyboardButton::f7(),
      KeyboardButton::f8(),
      KeyboardButton::f9(),
      KeyboardButton::f10(),
      KeyboardButton::num_lock(),
      KeyboardButton::scroll_lock(),
      KeyboardButton::ascii_key('7'),
      KeyboardButton::ascii_key('8'),
      KeyboardButton::ascii_key('9'),
      KeyboardButton::ascii_key('-'),
      KeyboardButton::ascii_key('4'),
      KeyboardButton::ascii_key('5'),
      KeyboardButton::ascii_key('6'),
      KeyboardButton::ascii_key('+'),
      KeyboardButton::ascii_key('1'),
      KeyboardButton::ascii_key('2'),
      KeyboardButton::ascii_key('3'),
      KeyboardButton::ascii_key('0'),
      KeyboardButton::ascii_key('.'),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      KeyboardButton::f11(),
      KeyboardButton::f12(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      KeyboardButton::enter(),
      KeyboardButton::rcontrol(),
      KeyboardButton::ascii_key('/'),
      KeyboardButton::print_screen(),
      KeyboardButton::ralt(),
      ButtonHandle::none(),
      KeyboardButton::home(),
      KeyboardButton::up(),
      KeyboardButton::page_up(),
      KeyboardButton::left(),
      KeyboardButton::right(),
      KeyboardButton::end(),
      KeyboardButton::down(),
      KeyboardButton::page_down(),
      KeyboardButton::insert(),
      KeyboardButton::del(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      KeyboardButton::pause(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      KeyboardButton::lmeta(),
      KeyboardButton::rmeta(),
      KeyboardButton::menu(),
    };
    return keyboard_map[code];

  } else if (code < 0x100) {
    return ButtonHandle::none();

  } else if ((code & 0xfff0) == BTN_MOUSE) {
    // The number for these is reversed in Panda.
    if (code == BTN_RIGHT) {
      return MouseButton::three();
    } else if (code == BTN_MIDDLE) {
      return MouseButton::two();
    } else {
      return MouseButton::button(code - BTN_MOUSE);
    }
  }

  switch (code) {
  case BTN_A:
    return GamepadButton::action_a();

  case BTN_B:
    return GamepadButton::action_b();

  case BTN_C:
    return GamepadButton::action_c();

  case BTN_X:
    return GamepadButton::action_x();

  case BTN_Y:
    return GamepadButton::action_y();

  case BTN_Z:
    return GamepadButton::action_z();

  case BTN_TL:
    return GamepadButton::lshoulder();

  case BTN_TR:
    return GamepadButton::rshoulder();

  case BTN_TL2:
    return GamepadButton::ltrigger();

  case BTN_TR2:
    return GamepadButton::rtrigger();

  case BTN_SELECT:
    return GamepadButton::back();

  case BTN_START:
    return GamepadButton::start();

  case BTN_MODE:
    return GamepadButton::guide();

  case BTN_THUMBL:
    return GamepadButton::lstick();

  case BTN_THUMBR:
    return GamepadButton::rstick();

  case BTN_TRIGGER_HAPPY1:
    return GamepadButton::dpad_left();

  case BTN_TRIGGER_HAPPY2:
    return GamepadButton::dpad_right();

  case BTN_TRIGGER_HAPPY3:
    return GamepadButton::dpad_up();

  case BTN_TRIGGER_HAPPY4:
    return GamepadButton::dpad_down();

  default:
    return ButtonHandle::none();
  }
}

#endif
