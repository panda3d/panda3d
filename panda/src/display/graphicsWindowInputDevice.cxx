// Filename: graphicsWindowInputDevice.cxx
// Created by:  drose (24May00)
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


#include "graphicsWindowInputDevice.h"
#include "graphicsWindow.h"
#include "mouseButton.h"
#include "keyboardButton.h"

TypeHandle GraphicsWindowInputDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::Constructor
//       Access: Private
//  Description: Defines a new InputDevice for the window.  Most
//               windows will have exactly one InputDevice: a
//               keyboard/mouse pair.  Some may also add joystick
//               data, or additional mice or something.
//
//               This private constructor is only used internally by
//               the named constructors, below.
////////////////////////////////////////////////////////////////////
GraphicsWindowInputDevice::
GraphicsWindowInputDevice(GraphicsWindow *host, const string &name, int flags) :
  InputDevice(name, DC_virtual, flags),
  _host(host)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::pointer_only
//       Access: Public
//  Description: This named constructor returns an input device that
//               only has a pointing device, no keyboard.
////////////////////////////////////////////////////////////////////
PT(GraphicsWindowInputDevice) GraphicsWindowInputDevice::
pointer_only(GraphicsWindow *host, const string &name) {
  return new GraphicsWindowInputDevice(host, name, IDF_has_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::keyboard_only
//       Access: Public
//  Description: This named constructor returns an input device that
//               only has a keyboard, no pointing device.
////////////////////////////////////////////////////////////////////
PT(GraphicsWindowInputDevice) GraphicsWindowInputDevice::
keyboard_only(GraphicsWindow *host, const string &name) {
  return new GraphicsWindowInputDevice(host, name, IDF_has_keyboard);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::pointer_and_keyboard
//       Access: Public
//  Description: This named constructor returns an input device that
//               has both a keyboard and pointer.
////////////////////////////////////////////////////////////////////
PT(GraphicsWindowInputDevice) GraphicsWindowInputDevice::
pointer_and_keyboard(GraphicsWindow *host, const string &name) {
  return new
    GraphicsWindowInputDevice(host, name, IDF_has_pointer | IDF_has_keyboard);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindowInputDevice::
GraphicsWindowInputDevice(const GraphicsWindowInputDevice &copy) {
  *this = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
operator = (const GraphicsWindowInputDevice &copy) {
  _host = copy._host;
  InputDevice::operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindowInputDevice::
~GraphicsWindowInputDevice() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::button_down
//       Access: Published
//  Description: Records that the indicated button has been depressed.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
button_down(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(button, ButtonEvent::T_down, time));
  _buttons_held.insert(button);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::button_resume_down
//       Access: Published
//  Description: Records that the indicated button was depressed
//               earlier, and we only just detected the event after
//               the fact.  This is mainly useful for tracking the
//               state of modifier keys.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
button_resume_down(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(button, ButtonEvent::T_resume_down, time));
  _buttons_held.insert(button);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::button_up
//       Access: Published
//  Description: Records that the indicated button has been released.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
button_up(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(button, ButtonEvent::T_up, time));
  _buttons_held.erase(button);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::keystroke
//       Access: Published
//  Description: Records that the indicated keystroke has been
//               generated.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
keystroke(int keycode, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(keycode, time));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::candidate
//       Access: Published
//  Description: Records that the indicated candidate string has been
//               highlighted.  This is used to implement IME support
//               for typing in international languages, especially
//               Chinese/Japanese/Korean.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
candidate(const wstring &candidate_string, size_t highlight_start,
          size_t highlight_end, size_t cursor_pos) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(candidate_string,
                                       highlight_start, highlight_end,
                                       cursor_pos));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::focus_lost
//       Access: Published
//  Description: This should be called when the window focus is lost,
//               so that we may miss upcoming button events
//               (especially "up" events) for the next period of time.
//               It generates keyboard and mouse "up" events for those
//               buttons that we previously sent unpaired "down"
//               events, so that the Panda application will believe
//               all buttons are now released.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
focus_lost(double time) {
  LightMutexHolder holder(_lock);
  ButtonsHeld::iterator bi;
  for (bi = _buttons_held.begin(); bi != _buttons_held.end(); ++bi) {
    _button_events->add_event(ButtonEvent(*bi, ButtonEvent::T_up, time));
  }
  _buttons_held.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::raw_button_down
//       Access: Published
//  Description: Records that the indicated button has been depressed.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
raw_button_down(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(button, ButtonEvent::T_raw_down, time));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::raw_button_up
//       Access: Published
//  Description: Records that the indicated button has been released.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
raw_button_up(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events->add_event(ButtonEvent(button, ButtonEvent::T_raw_up, time));
}
