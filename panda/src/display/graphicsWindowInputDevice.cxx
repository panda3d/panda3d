// Filename: graphicsWindowInputDevice.cxx
// Created by:  drose (24May00)
// 
////////////////////////////////////////////////////////////////////

#include "graphicsWindowInputDevice.h"
#include <mouseButton.h>
#include <keyboardButton.h>


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
  // We'll define the mouse buttons and the traditional modifier keys
  // as the default modifiers.  Individual GraphicsWindows can change
  // this if they want.

  _mods.add_button(MouseButton::one());
  _mods.add_button(MouseButton::two());
  _mods.add_button(MouseButton::three());
  _mods.add_button(KeyboardButton::shift());
  _mods.add_button(KeyboardButton::control());
  _mods.add_button(KeyboardButton::alt());
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
  _mods(copy._mods),
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
  _mods = copy._mods;
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
button_down(ButtonHandle button) {
  _button_events.push_back(ButtonEvent(button, true, _mods));
  _mods.button_down(button);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::button_up
//       Access: Public
//  Description: Records that the indicated button has been released.
////////////////////////////////////////////////////////////////////
void GraphicsWindowInputDevice::
button_up(ButtonHandle button) {
  _mods.button_up(button);
  _button_events.push_back(ButtonEvent(button, false, _mods));
}
