/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file evdevInputDevice.cxx
 * @author rdb
 * @date 2015-08-24
 */

#include "evdevInputDevice.h"

#ifdef PHAVE_LINUX_INPUT_H

#include "gamepadButton.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "inputDeviceManager.h"

#include <fcntl.h>
#include <linux/input.h>

#define test_bit(bit, array) ((array)[(bit)>>3] & (1<<((bit)&7)))

static InputDevice::ControlAxis axis_map[] = {
  // ABS_X = 0x00
  InputDevice::C_left_x,
  // ABS_Y = 0x01
  InputDevice::C_left_y,
  // ABS_Z = 0x02
  InputDevice::C_left_trigger,
  // ABS_RX = 0x03
  InputDevice::C_right_x,
  // ABS_RY = 0x04
  InputDevice::C_right_y,
  // ABS_RZ = 0x05
  InputDevice::C_right_trigger,
  // ABS_THROTTLE = 0x06
  InputDevice::C_throttle,
  // ABS_RUDDER = 0x07
  InputDevice::C_rudder,
  // ABS_WHEEL = 0x08
  InputDevice::C_wheel,
  // ABS_GAS = 0x09
  InputDevice::C_accelerator,
  // ABS_BRAKE = 0x0a
  InputDevice::C_brake,

  InputDevice::C_none,
  InputDevice::C_none,
  InputDevice::C_none,
  InputDevice::C_none,
  InputDevice::C_none,

  // ABS_HAT0X = 0x10
  InputDevice::C_hat_x,
  // ABS_HAT0Y = 0x11
  InputDevice::C_hat_y,

  // ABS_HAT1X = 0x12
  // ABS_HAT1Y = 0x13
  // ABS_HAT2X = 0x14
  // ABS_HAT2Y = 0x15
  // ABS_HAT3X = 0x16
  // ABS_HAT3Y = 0x17
  // ABS_PRESSURE = 0x18
  // ABS_DISTANCE = 0x19
  // ABS_TILT_X = 0x1a
  // ABS_TILT_Y = 0x1b
  // ABS_TOOL_WIDTH = 0x1c
};

TypeHandle EvdevInputDevice::_type_handle;

/**
 * Creates a new device representing the evdev device with the given index.
 */
EvdevInputDevice::
EvdevInputDevice(int index) :
  _index(index),
  _fd(-1),
  _can_write(false),
  _ff_id(-1),
  _ff_playing(false),
  _ff_strong(-1),
  _ff_weak(-1),
  _dpad_x_axis(-1),
  _dpad_y_axis(-1),
  _dpad_left_button(-1),
  _dpad_up_button(-1) {

  char path[64];
  sprintf(path, "/dev/input/event%d", index);

  _fd = open(path, O_RDWR | O_NONBLOCK);
  if (_fd >= 0) {
    _can_write = true;
  } else {
    // On failure, open device as read-only.
    _fd = open(path, O_RDONLY | O_NONBLOCK);
  }

  if (_fd >= 0) {
    init_device();
  } else {
    _is_connected = false;
    device_cat.error()
      << "Opening raw input device: " << strerror(errno) << " " << path << "\n";
  }
}

/**
 *
 */
EvdevInputDevice::
~EvdevInputDevice() {
  if (_fd != -1) {
    if (_ff_id != -1) {
      // Remove force-feedback effect.
      do_set_vibration(0, 0);
      ioctl(_fd, EVIOCRMFF, _ff_id);
      _ff_id = -1;
    }

    close(_fd);
    _fd = -1;
  }
}

/**
 * Sets the vibration strength.  The first argument controls a low-frequency
 * motor, if present, and the latter controls a high-frequency motor.
 * The values are within the 0-1 range.
 */
void EvdevInputDevice::
do_set_vibration(double strong, double weak) {
  if (_fd == -1 || !_can_write) {
    return;
  }

  int strong_level = strong * 0xffff;
  int weak_level = weak * 0xffff;

  if (strong_level == _ff_strong && weak_level == _ff_weak) {
    // No change.
    return;
  }

  // Upload the new effect parameters.  Do this even if we are about
  // to stop the effect, because some drivers don't respond to simply
  // stopping the effect.
  struct ff_effect effect;
  effect.type = FF_RUMBLE;
  effect.id = _ff_id;
  effect.direction = 0;
  effect.trigger.button = 0;
  effect.trigger.interval = 0;
  effect.replay.length = 0;
  effect.replay.delay = 0;
  effect.u.rumble.strong_magnitude = strong_level;
  effect.u.rumble.weak_magnitude = weak_level;

  if (ioctl(_fd, EVIOCSFF, &effect) < 0) {
    return;
  } else {
    _ff_id = effect.id;
    _ff_strong = strong_level;
    _ff_weak = weak_level;
  }

  if (!_ff_playing) {
    // Start the effect.  We could pass 0 as value to stop the effect
    // when a level of 0 is requested, but my driver seems to ignore it.
    _ff_playing = true;

    struct input_event play;
    play.type = EV_FF;
    play.code = _ff_id;
    play.value = 1;

    if (write(_fd, &play, sizeof(play)) < 0) {
      device_cat.warning()
        << "Failed to write force-feedback event: " << strerror(errno) << "\n";
    }
  }
}

/**
 * Polls the input device for new activity, to ensure it contains the latest
 * events.  This will only have any effect for some types of input devices;
 * others may be updated automatically, and this method will be a no-op.
 */
void EvdevInputDevice::
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
 * Reads basic properties from the device.
 */
bool EvdevInputDevice::
init_device() {
  nassertr(_fd >= 0, false);

  uint8_t evtypes[(EV_CNT + 7) >> 3];
  memset(evtypes, 0, sizeof(evtypes));
  char name[128];
  if (ioctl(_fd, EVIOCGNAME(sizeof(name)), name) < 0 ||
      ioctl(_fd, EVIOCGBIT(0, sizeof(evtypes)), evtypes) < 0) {
    close(_fd);
    _fd = -1;
    _is_connected = false;
    device_cat.error() << "Opening raw input device: ioctl failed\n";
    return false;
  }

  _name.assign(name);
  //cerr << "##### Now initializing device " << name << "\n";

  struct input_id id;
  if (ioctl(_fd, EVIOCGID, &id) >= 0) {
    _vendor_id = id.vendor;
    _product_id = id.product;
  }

  bool all_values_zero = true;
  bool emulate_dpad = true;

  bool has_keys = false;
  bool has_axes = false;

  uint8_t keys[(KEY_CNT + 7) >> 3];
  if (test_bit(EV_KEY, evtypes)) {
    // Check which buttons are on the device.
    memset(keys, 0, sizeof(keys));
    ioctl(_fd, EVIOCGBIT(EV_KEY, sizeof(keys)), keys);
    has_keys = true;

    if (test_bit(KEY_A, keys) && test_bit(KEY_Z, keys)) {
      _flags |= IDF_has_keyboard;
    }
  }

  int num_bits = 0;
  uint8_t axes[(ABS_CNT + 7) >> 3];
  if (test_bit(EV_ABS, evtypes)) {
    // Check which axes are on the device.
    memset(axes, 0, sizeof(axes));
    num_bits = ioctl(_fd, EVIOCGBIT(EV_ABS, sizeof(axes)), axes) << 3;
    has_axes = true;
  }


  // Try to detect which type of device we have here
  int device_scores[DC_COUNT];
  memset(device_scores, 0, sizeof(device_scores));

  // Test for specific keys
  if (test_bit(BTN_GAMEPAD, keys)) {
    device_scores[DC_gamepad] += 5;
    device_scores[DC_steering_wheel] += 5;
    device_scores[DC_flight_stick] += 5;
  }

  if (test_bit(ABS_WHEEL, axes) && test_bit(ABS_GAS, axes) && test_bit(ABS_BRAKE, axes)) {
    device_scores[DC_steering_wheel] += 10;
  }
  if (test_bit(BTN_GEAR_DOWN, keys) && test_bit(BTN_GEAR_UP, keys)) {
    device_scores[DC_steering_wheel] += 10;
  }
  if (test_bit(BTN_JOYSTICK, keys)) {
    device_scores[DC_flight_stick] += 10;
  }
  if (test_bit(BTN_MOUSE, keys)) {
    device_scores[DC_mouse] += 20;
  }
  uint8_t unknown_keys[] = {KEY_POWER};
  for (int i = 0; i < 1; i++) {
    if (test_bit(unknown_keys[i], keys)) {
      if (unknown_keys[i] == KEY_POWER) {
      }
      device_scores[DC_unknown] += 20;
    }
  }
  if (_flags & IDF_has_keyboard) {
    device_scores[DC_keyboard] += 20;
  }

  // Test for specific name tags
  string lowercase_name = _name;
  for(int x=0; x<_name.length(); x++) {
    lowercase_name[x]=tolower(lowercase_name[x]);
  }
  if (lowercase_name.find("gamepad") != string::npos) {
    device_scores[DC_gamepad] += 10;
  }
  if (lowercase_name.find("wheel") != string::npos) {
    device_scores[DC_steering_wheel] += 10;
  }
  if (lowercase_name.find("mouse") != string::npos || lowercase_name.find("touchpad") != string::npos) {
    device_scores[DC_mouse] += 10;
  }
  if (lowercase_name.find("keyboard") != string::npos) {
    device_scores[DC_keyboard] += 10;
  }
  // List of lowercase names that occur in unknown devices
  string unknown_names[] = {"video bus", "power button", "sleep button"};
  for(int i = 0; i < 3; i++) {
    if (lowercase_name.find(unknown_names[i]) != string::npos) {
      device_scores[DC_unknown] += 20;
    }
  }

  // Check which device type got the most points
  size_t highest_score = 0;
  for (size_t i = 0; i < DC_COUNT; i++) {
    if (device_scores[i] > highest_score) {
      highest_score = device_scores[i];
      _device_class = (DeviceClass)i;
    }
  }
  //cerr << "Found highscore class " << _device_class << " with this score: " << highest_score << "\n";

  if (_device_class != DC_gamepad) {
    emulate_dpad = false;
  }

  if (has_keys) {
    // Also check whether the buttons are currently pressed.
    uint8_t states[(KEY_CNT + 7) >> 3];
    memset(states, 0, sizeof(states));
    ioctl(_fd, EVIOCGKEY(sizeof(states)), states);

    int bi = 0;
    for (int i = 0; i < KEY_CNT; ++i) {
      if (test_bit(i, keys)) {
        set_button_map(bi, map_button(i));
        //cerr << "Button " << bi << " is mapped by the driver to " << i << "\n";
        if (test_bit(i, states)) {
          _buttons[bi].state = S_down;
          all_values_zero = false;
        } else {
          _buttons[bi].state = S_up;
        }
        if (_buttons[bi].handle == GamepadButton::dpad_left()) {
          emulate_dpad = false;
        }
        ++bi;
      }
    }
  }

  if (has_axes) {
    AxisRange range;
    range._scale = 1.0;
    range._bias = 0.0;
    _axis_ranges.resize(ABS_CNT, range);


    for (int i = 0; i < num_bits; ++i) {
      if (test_bit(i, axes)) {
        if (i >= ABS_HAT0X) {
          set_control_map(i, C_none);

          // Emulate D-Pad buttons if necessary.
          if (i == ABS_HAT0X && emulate_dpad) {
            _dpad_x_axis = i;
            _dpad_left_button = (int)_buttons.size();
            _buttons.push_back(ButtonState(GamepadButton::dpad_left()));
            _buttons.push_back(ButtonState(GamepadButton::dpad_right()));

          } else if (i == ABS_HAT0Y && emulate_dpad) {
            _dpad_y_axis = i;
            _dpad_up_button = (int)_buttons.size();
            _buttons.push_back(ButtonState(GamepadButton::dpad_up()));
            _buttons.push_back(ButtonState(GamepadButton::dpad_down()));
          }
        } else {
          set_control_map(i, axis_map[i]);
          //cerr << "Axis " << axis_map[i] << " is mapped by the driver to " << i << "\n";
        }

        // Check the initial value and ranges.
        struct input_absinfo absinfo;
        if (ioctl(_fd, EVIOCGABS(i), &absinfo) >= 0) {
          double factor, bias;
          if (absinfo.minimum < 0) {
            // Centered, eg. for sticks.
            factor = 2.0 / (absinfo.maximum - absinfo.minimum);
            bias = (absinfo.maximum + absinfo.minimum) / (double)(absinfo.minimum - absinfo.maximum);
          } else {
            // 0-based, eg. for triggers.
            factor = 1.0 / absinfo.maximum;
            bias = 0.0;
          }

          // Flip Y axis to match Windows implementation.
          if (i == ABS_Y || i == ABS_RY) {
            factor = -factor;
            bias = -bias;
          }

          _axis_ranges[i]._scale = factor;
          _axis_ranges[i]._bias = bias;
          _controls[i].state = fma(absinfo.value, factor, bias);

          if (absinfo.value != 0) {
            all_values_zero = false;
          }
        }
      }
    }
  }

  if (test_bit(EV_REL, evtypes)) {
    _flags |= IDF_has_pointer;
  }

  if (test_bit(EV_FF, evtypes)) {
    uint8_t effects[(FF_CNT + 7) >> 3];
    memset(effects, 0, sizeof(effects));
    ioctl(_fd, EVIOCGBIT(EV_FF, sizeof(effects)), effects);

    if (test_bit(FF_RUMBLE, effects)) {
      if (_can_write) {
        _flags |= IDF_has_vibration;
      } else {
        // Let the user know what he's missing out on.
        device_cat.warning()
          << "/dev/input/event" << _index << " is not writable, vibration "
          << "effects will be unavailable.\n";
      }
    }
  }

  char path[64];
  char buffer[256];
  sprintf(path, "/sys/class/input/event%d/device/device/../product", _index);
  FILE *f = fopen(path, "r");
  if (f) {
    if (fgets(buffer, sizeof(buffer), f) != NULL) {
      buffer[strcspn(buffer, "\r\n")] = 0;
      if (buffer[0] != 0) {
        _name.assign(buffer);
      }
    }
    fclose(f);
  }
  sprintf(path, "/sys/class/input/event%d/device/device/../manufacturer", _index);
  f = fopen(path, "r");
  if (f) {
    if (fgets(buffer, sizeof(buffer), f) != NULL) {
      buffer[strcspn(buffer, "\r\n")] = 0;
      _manufacturer.assign(buffer);
    }
    fclose(f);
  }
  sprintf(path, "/sys/class/input/event%d/device/device/../serial", _index);
  f = fopen(path, "r");
  if (f) {
    if (fgets(buffer, sizeof(buffer), f) != NULL) {
      buffer[strcspn(buffer, "\r\n")] = 0;
      _serial_number.assign(buffer);
    }
    fclose(f);
  }

  // Special-case fix for Xbox 360 Wireless Receiver: the Linux kernel
  // driver always reports 4 connected gamepads, regardless of the number
  // of gamepads actually present.  This hack partially remedies this.
  if (all_values_zero && _vendor_id == 0x045e && _product_id == 0x0719) {
    _is_connected = false;
  } else {
    _is_connected = true;
  }
  return true;
}

/**
 * Reads a number of events from the device.  Returns true if events were read,
 * meaning this function should keep being called until it returns false.
 */
bool EvdevInputDevice::
process_events() {
  // Read 8 events at a time.
  struct input_event events[8];

  int n_read = read(_fd, events, sizeof(events));
  if (n_read < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // No data available for now.

    } else if (errno == ENODEV || errno == EINVAL) {
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

  n_read /= sizeof(struct input_event);

  int x = _pointer_data.get_x();
  int y = _pointer_data.get_y();
  bool have_pointer = false;
  double time = ClockObject::get_global_clock()->get_frame_time();
  ButtonHandle button;

  // It seems that some devices send a single EV_SYN event when being
  // unplugged.  Boo.  Ignore it.
  if (n_read == 1 && events[0].code == EV_SYN) {
    return false;
  }

  for (int i = 0; i < n_read; ++i) {
    int code = events[i].code;

    switch (events[i].type) {
    case EV_SYN:
      break;

    case EV_REL:
      if (code == REL_X) x += events[i].value;
      if (code == REL_Y) y += events[i].value;
      have_pointer = true;
      break;

    case EV_ABS:
      if (code == _dpad_x_axis) {
        set_button_state(_dpad_left_button, events[i].value < 0);
        set_button_state(_dpad_left_button+1, events[i].value > 0);
      } else if (code == _dpad_y_axis) {
        set_button_state(_dpad_up_button, events[i].value < 0);
        set_button_state(_dpad_up_button+1, events[i].value > 0);
      }
      set_control_state(code, fma(events[i].value, _axis_ranges[code]._scale, _axis_ranges[code]._bias));
      break;

    case EV_KEY:
      button = map_button(code);
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

/**
 * Static function to map an evdev code to a ButtonHandle.
 */
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

  case BTN_1:
    return GamepadButton::action_1();

  case BTN_2:
    return GamepadButton::action_2();

  case BTN_SELECT:
  case KEY_PREVIOUS:
    return GamepadButton::back();

  case BTN_START:
  case KEY_NEXT:
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
