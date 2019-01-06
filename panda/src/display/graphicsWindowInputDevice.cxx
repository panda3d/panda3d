/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsWindowInputDevice.cxx
 * @author drose
 * @date 2000-05-24
 */

#include "graphicsWindowInputDevice.h"
#include "graphicsWindow.h"
#include "mouseButton.h"
#include "keyboardButton.h"

TypeHandle GraphicsWindowInputDevice::_type_handle;

using std::string;

/**
 * Defines a new InputDevice for the window.  Most windows will have exactly
 * one InputDevice: a keyboard/mouse pair.  Some may also add joystick data,
 * or additional mice or something.
 *
 * This private constructor is only used internally by the named constructors,
 * below.
 */
GraphicsWindowInputDevice::
GraphicsWindowInputDevice(GraphicsWindow *host, const string &name, bool pointer, bool keyboard) :
  InputDevice(name, DeviceClass::virtual_device)
{
  if (pointer) {
    enable_feature(Feature::pointer);
    add_pointer(PointerType::mouse, 0);
  }
  if (keyboard) {
    enable_feature(Feature::keyboard);
  }
}

/**
 * This named constructor returns an input device that only has a pointing
 * device, no keyboard.
 */
PT(GraphicsWindowInputDevice) GraphicsWindowInputDevice::
pointer_only(GraphicsWindow *host, const string &name) {
  return new GraphicsWindowInputDevice(host, name, true, false);
}

/**
 * This named constructor returns an input device that only has a keyboard, no
 * pointing device.
 */
PT(GraphicsWindowInputDevice) GraphicsWindowInputDevice::
keyboard_only(GraphicsWindow *host, const string &name) {
  return new GraphicsWindowInputDevice(host, name, false, true);
}

/**
 * This named constructor returns an input device that has both a keyboard and
 * pointer.
 */
PT(GraphicsWindowInputDevice) GraphicsWindowInputDevice::
pointer_and_keyboard(GraphicsWindow *host, const string &name) {
  return new GraphicsWindowInputDevice(host, name, true, true);
}

/**
 * Records that the indicated button has been depressed.
 */
void GraphicsWindowInputDevice::
button_down(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(button, ButtonEvent::T_down, time));
  _buttons_held.insert(button);
}

/**
 * Records that the indicated button was depressed earlier, and we only just
 * detected the event after the fact.  This is mainly useful for tracking the
 * state of modifier keys.
 */
void GraphicsWindowInputDevice::
button_resume_down(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(button, ButtonEvent::T_resume_down, time));
  _buttons_held.insert(button);
}

/**
 * Records that the indicated button has been released.
 */
void GraphicsWindowInputDevice::
button_up(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(button, ButtonEvent::T_up, time));
  _buttons_held.erase(button);
}

/**
 * Records that the indicated keystroke has been generated.
 */
void GraphicsWindowInputDevice::
keystroke(int keycode, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(keycode, time));
}

/**
 * Records that the indicated candidate string has been highlighted.  This is
 * used to implement IME support for typing in international languages,
 * especially Chinese/Japanese/Korean.
 */
void GraphicsWindowInputDevice::
candidate(const std::wstring &candidate_string, size_t highlight_start,
          size_t highlight_end, size_t cursor_pos) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(candidate_string,
                                       highlight_start, highlight_end,
                                       cursor_pos));
}

/**
 * This should be called when the window focus is lost, so that we may miss
 * upcoming button events (especially "up" events) for the next period of
 * time.  It generates keyboard and mouse "up" events for those buttons that
 * we previously sent unpaired "down" events, so that the Panda application
 * will believe all buttons are now released.
 */
void GraphicsWindowInputDevice::
focus_lost(double time) {
  LightMutexHolder holder(_lock);
  ButtonsHeld::iterator bi;
  for (bi = _buttons_held.begin(); bi != _buttons_held.end(); ++bi) {
    _button_events->add_event(ButtonEvent(*bi, ButtonEvent::T_up, time));
  }
  _buttons_held.clear();
}

/**
 * Records that the indicated button has been depressed.
 */
void GraphicsWindowInputDevice::
raw_button_down(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(button, ButtonEvent::T_raw_down, time));
}

/**
 * Records that the indicated button has been released.
 */
void GraphicsWindowInputDevice::
raw_button_up(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(button, ButtonEvent::T_raw_up, time));
}

/**
 * To be called by a particular kind of GraphicsWindow to indicate that the
 * pointer is within the window, at the given pixel coordinates.
 */
void GraphicsWindowInputDevice::
set_pointer_in_window(double x, double y, double time) {
  LightMutexHolder holder(_lock);
  PointerData data = _pointers[0];
  data._id = 0;
  data._type = PointerType::mouse;
  data._xpos = x;
  data._ypos = y;
  data._in_window = true;
  InputDevice::update_pointer(data, time);
}

/**
 * To be called by a particular kind of GraphicsWindow to indicate that the
 * pointer is no longer within the window.
 */
void GraphicsWindowInputDevice::
set_pointer_out_of_window(double time) {
  LightMutexHolder holder(_lock);
  if (_pointers.empty()) {
    return;
  }

  _pointers[0]._in_window = false;
  _pointers[0]._pressure = 0.0;

  if (_enable_pointer_events) {
    int seq = _event_sequence++;
    if (_pointer_events.is_null()) {
      _pointer_events = new PointerEventList();
    }
    _pointer_events->add_event(_pointers[0]._in_window,
                               _pointers[0]._xpos,
                               _pointers[0]._ypos,
                               seq, time);
  }
}
