// Filename: graphicsWindowInputDevice.cxx
// Created by:  drose (24May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


#include "graphicsWindowInputDevice.h"
#include "mouseButton.h"
#include "keyboardButton.h"

#define EXPCL EXPCL_PANDA
#define EXPTP EXPTP_PANDA
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
GraphicsWindowInputDevice(const string &name, int flags) :
  _name(name),
  _flags(flags)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::pointer_only
//       Access: Public
//  Description: This named constructor returns an input device that
//               only has a pointing device, no keyboard.
////////////////////////////////////////////////////////////////////
GraphicsWindowInputDevice GraphicsWindowInputDevice::
pointer_only(const string &name) {
  return GraphicsWindowInputDevice(name, IDF_has_pointer);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::keyboard_only
//       Access: Public
//  Description: This named constructor returns an input device that
//               only has a keyboard, no pointing device.
////////////////////////////////////////////////////////////////////
GraphicsWindowInputDevice GraphicsWindowInputDevice::
keyboard_only(const string &name) {
  return GraphicsWindowInputDevice(name, IDF_has_keyboard);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::pointer_and_keyboard
//       Access: Public
//  Description: This named constructor returns an input device that
//               has both a keyboard and pointer.
////////////////////////////////////////////////////////////////////
GraphicsWindowInputDevice GraphicsWindowInputDevice::
pointer_and_keyboard(const string &name) {
  return
    GraphicsWindowInputDevice(name, IDF_has_pointer | IDF_has_keyboard);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindowInputDevice::
GraphicsWindowInputDevice(const GraphicsWindowInputDevice &copy) :
  _name(copy._name),
  _flags(copy._flags),
  _mouse_data(copy._mouse_data),
  _button_events(copy._button_events)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
operator = (const GraphicsWindowInputDevice &copy) {
  _name = copy._name;
  _flags = copy._flags;
  _mouse_data = copy._mouse_data;
  _button_events = copy._button_events;
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
  ButtonEvent be = _button_events.front();
  _button_events.pop_front();
  return be;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::button_down
//       Access: Public
//  Description: Records that the indicated button has been depressed.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
button_down(ButtonHandle button, double time) {
  _button_events.push_back(ButtonEvent(button, ButtonEvent::T_down, time));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::button_resume_down
//       Access: Public
//  Description: Records that the indicated button was depressed
//               earlier, and we only just detected the event after
//               the fact.  This is mainly useful for tracking the
//               state of modifier keys.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
button_resume_down(ButtonHandle button, double time) {
  _button_events.push_back(ButtonEvent(button, ButtonEvent::T_resume_down, time));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::button_up
//       Access: Public
//  Description: Records that the indicated button has been released.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
button_up(ButtonHandle button, double time) {
  _button_events.push_back(ButtonEvent(button, ButtonEvent::T_up, time));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::keystroke
//       Access: Public
//  Description: Records that the indicated keystroke has been
//               generated.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
keystroke(int keycode, double time) {
  _button_events.push_back(ButtonEvent(keycode, time));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::candidate
//       Access: Public
//  Description: Records that the indicated candidate string has been
//               highlighted.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
candidate(const wstring &candidate_string, size_t highlight_start, 
          size_t highlight_end, double time) {
  _button_events.push_back(ButtonEvent(candidate_string, 
                                       highlight_start, highlight_end));
}
