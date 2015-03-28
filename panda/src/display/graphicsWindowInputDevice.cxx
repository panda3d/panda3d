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

#define EXPCL EXPCL_PANDA_DISPLAY
#define EXPTP EXPTP_PANDA_DISPLAY
#define TYPE GraphicsWindowInputDevice
#define NAME vector_GraphicsWindowInputDevice

#include "vector_src.cxx"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

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
  _host(host),
  _name(name),
  _flags(flags),
  _device_index(0),
  _event_sequence(0),
  _pointer_mode_enable(false),
  _pointer_speed(1.0),
  _enable_pointer_events(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::pointer_only
//       Access: Public
//  Description: This named constructor returns an input device that
//               only has a pointing device, no keyboard.
////////////////////////////////////////////////////////////////////
GraphicsWindowInputDevice GraphicsWindowInputDevice::
pointer_only(GraphicsWindow *host, const string &name) {
  return GraphicsWindowInputDevice(host, name, IDF_has_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::keyboard_only
//       Access: Public
//  Description: This named constructor returns an input device that
//               only has a keyboard, no pointing device.
////////////////////////////////////////////////////////////////////
GraphicsWindowInputDevice GraphicsWindowInputDevice::
keyboard_only(GraphicsWindow *host, const string &name) {
  return GraphicsWindowInputDevice(host, name, IDF_has_keyboard);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::pointer_and_keyboard
//       Access: Public
//  Description: This named constructor returns an input device that
//               has both a keyboard and pointer.
////////////////////////////////////////////////////////////////////
GraphicsWindowInputDevice GraphicsWindowInputDevice::
pointer_and_keyboard(GraphicsWindow *host, const string &name) {
  return
    GraphicsWindowInputDevice(host, name, IDF_has_pointer | IDF_has_keyboard);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindowInputDevice::
GraphicsWindowInputDevice(const GraphicsWindowInputDevice &copy)
{
    *this = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
operator = (const GraphicsWindowInputDevice &copy)
{
  LightMutexHolder holder(_lock);
  LightMutexHolder holder1(copy._lock);
  _host = copy._host;
  _name = copy._name;
  _flags = copy._flags;
  _device_index = copy._device_index;
  _event_sequence = copy._event_sequence;
  _pointer_mode_enable = copy._pointer_mode_enable;
  _pointer_speed = copy._pointer_speed;
  _enable_pointer_events = copy._enable_pointer_events;
  _mouse_data = copy._mouse_data;
  _true_mouse_data = copy._true_mouse_data;
  _button_events = copy._button_events;
  _pointer_events = copy._pointer_events;
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
//     Function: GraphicsWindowInputDevice::has_button_event
//       Access: Public
//  Description: Returns true if this device has a pending button
//               event (a mouse button or keyboard button down/up),
//               false otherwise.  If this returns true, the
//               particular event may be extracted via
//               get_button_event().
////////////////////////////////////////////////////////////////////
bool GraphicsWindowInputDevice::
has_button_event() const {
  LightMutexHolder holder(_lock);
  return !_button_events.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::get_button_event
//       Access: Public
//  Description: Assuming a previous call to has_button_event()
//               returned true, this returns the pending button event.
////////////////////////////////////////////////////////////////////
ButtonEvent GraphicsWindowInputDevice::
get_button_event() {
  LightMutexHolder holder(_lock);
  ButtonEvent be = _button_events.front();
  _button_events.pop_front();
  return be;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::has_pointer_event
//       Access: Public
//  Description: Returns true if this device has a pending pointer
//               event (a mouse movement), or false otherwise.  If
//               this returns true, the particular event may be
//               extracted via get_pointer_event().
////////////////////////////////////////////////////////////////////
bool GraphicsWindowInputDevice::
has_pointer_event() const {
  LightMutexHolder holder(_lock);
  return (_pointer_events != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::get_pointer_events
//       Access: Public
//  Description: Returns a PointerEventList containing all the recent
//               pointer events.
////////////////////////////////////////////////////////////////////
PT(PointerEventList) GraphicsWindowInputDevice::
get_pointer_events() {
  LightMutexHolder holder(_lock);
  PT(PointerEventList) result = _pointer_events;
  _pointer_events = 0;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::enable_pointer_mode
//       Access: Public
//  Description: There are two modes: raw mode, and pointer mode.
//               In pointer mode, the mouse stops when it reaches the
//               edges of the window.  In raw mode, the mouse ignores
//               the screen boundaries and can continue indefinitely,
//               even into negative coordinates.  In raw mode, each
//               "blip" from the mouse hardware corresponds to a
//               change of 1 unit in the mouse's (x,y) coordinate.
//               In pointer mode, a variety of speed adjustment factors
//               and concepts like "mouse acceleration" may be applied.
//
//               Mouse zero represents the system mouse pointer.  This
//               is by definition a pointer, not a raw mouse.  It is
//               an error to try to enable or disable pointer mode on
//               mouse zero.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
enable_pointer_mode(double speed) {
  LightMutexHolder holder(_lock);
  nassertv(_device_index != 0);
  _pointer_mode_enable = true;
  _pointer_speed = speed;
  _mouse_data._xpos = _host->get_x_size() * 0.5;
  _mouse_data._ypos = _host->get_y_size() * 0.5;
  _mouse_data._in_window = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::disable_pointer_mode
//       Access: Public
//  Description: see enable_pointer_mode.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
disable_pointer_mode() {
  LightMutexHolder holder(_lock);
  nassertv(_device_index != 0);
  _pointer_mode_enable = false;
  _pointer_speed = 1.0;
  _mouse_data = _true_mouse_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_pointer
//       Access: Published
//  Description: Records that a mouse movement has taken place.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
set_pointer(bool inwin, double x, double y, double time) {
  LightMutexHolder holder(_lock);

  double delta_x = x - _true_mouse_data._xpos;
  double delta_y = y - _true_mouse_data._ypos;
  _true_mouse_data._in_window = inwin;
  _true_mouse_data._xpos = x;
  _true_mouse_data._ypos = y;

  if (_pointer_mode_enable) {
    double pointer_x = _mouse_data._xpos;
    double pointer_y = _mouse_data._ypos;
    pointer_x += (delta_x * _pointer_speed);
    pointer_y += (delta_y * _pointer_speed);
    double xhi = _host->get_x_size();
    double yhi = _host->get_y_size();
    if (pointer_x < 0.0) pointer_x = 0.0;
    if (pointer_y < 0.0) pointer_y = 0.0;
    if (pointer_x > xhi) pointer_x = xhi;
    if (pointer_y > yhi) pointer_y = yhi;
    _mouse_data._in_window = true;
    _mouse_data._xpos = pointer_x;
    _mouse_data._ypos = pointer_y;
  } else {
    _mouse_data = _true_mouse_data;
  }

  if (_enable_pointer_events) {
    int seq = _event_sequence++;
    if (_pointer_events == 0) {
      _pointer_events = new PointerEventList();
    }
    _pointer_events->add_event(_mouse_data._in_window,
                               _mouse_data._xpos,
                               _mouse_data._ypos,
                               seq, time);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::button_down
//       Access: Published
//  Description: Records that the indicated button has been depressed.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
button_down(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events.push_back(ButtonEvent(button, ButtonEvent::T_down, time));
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
  _button_events.push_back(ButtonEvent(button, ButtonEvent::T_resume_down, time));
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
  _button_events.push_back(ButtonEvent(button, ButtonEvent::T_up, time));
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
  _button_events.push_back(ButtonEvent(keycode, time));
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
  _button_events.push_back(ButtonEvent(candidate_string,
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
    _button_events.push_back(ButtonEvent(*bi, ButtonEvent::T_up, time));
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
  _button_events.push_back(ButtonEvent(button, ButtonEvent::T_raw_down, time));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::raw_button_up
//       Access: Published
//  Description: Records that the indicated button has been released.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
raw_button_up(ButtonHandle button, double time) {
  LightMutexHolder holder(_lock);
  _button_events.push_back(ButtonEvent(button, ButtonEvent::T_raw_up, time));
}
