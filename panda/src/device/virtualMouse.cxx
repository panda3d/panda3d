// Filename: virtualMouse.cxx
// Created by:  drose (13Dec01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "virtualMouse.h"

TypeHandle VirtualMouse::_type_handle;

TypeHandle VirtualMouse::_pixel_xyz_type;
TypeHandle VirtualMouse::_xyz_type;
TypeHandle VirtualMouse::_button_events_type;


////////////////////////////////////////////////////////////////////
//     Function: VirtualMouse::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
VirtualMouse::
VirtualMouse(const string &name) :
  DataNode(name)
{
  _pixel_xyz = new Vec3DataTransition(LPoint3f(0.0f, 0.0f, 0.0f));
  _xyz = new Vec3DataTransition(LPoint3f(0.0f, 0.0f, 0.0f));
  _button_events = new ButtonEventDataTransition();

  _got_mouse_attrib.set_transition(_pixel_xyz_type, _pixel_xyz);
  _got_mouse_attrib.set_transition(_xyz_type, _xyz);
  _got_mouse_attrib.set_transition(_button_events_type, _button_events);

  _no_mouse_attrib.set_transition(_button_events_type, _button_events);

  _mouse_x = 0;
  _mouse_y = 0;
  _win_width = 100;
  _win_height = 100;
  _mouse_on = false;
  _next_button_events = new ButtonEventDataTransition();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualMouse::set_mouse_pos
//       Access: Published
//  Description: Sets the current mouse pixel location, where (0,0) is
//               the upper left, and (width-1, height-1) is the lower
//               right pixel of the virtual window.
////////////////////////////////////////////////////////////////////
void VirtualMouse::
set_mouse_pos(int x, int y) {
  _mouse_x = x;
  _mouse_y = y;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualMouse::set_window_size
//       Access: Published
//  Description: Sets the size of the "window" in which the mouse
//               rolls.  This changes the meaning of the values passed
//               to set_mouse_pos().
////////////////////////////////////////////////////////////////////
void VirtualMouse::
set_window_size(int width, int height) {
  _win_width = width;
  _win_height = height;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualMouse::set_mouse_on
//       Access: Published
//  Description: Sets whether the mouse should appear to be within the
//               window or not.  If this is true, the mouse is within
//               the window; if false, the mouse is not within the
//               window (and set_mouse_pos() means nothing).
////////////////////////////////////////////////////////////////////
void VirtualMouse::
set_mouse_on(bool flag) {
  _mouse_on = flag;
}
  
////////////////////////////////////////////////////////////////////
//     Function: VirtualMouse::press_button
//       Access: Published
//  Description: Simulates a mouse or keyboard button being depressed.
//               This should be followed up by a call to
//               release_button() sometime later (possibly
//               immediately).
////////////////////////////////////////////////////////////////////
void VirtualMouse::
press_button(ButtonHandle button) {
  _next_button_events->push_back(ButtonEvent(button, true));
}
  
////////////////////////////////////////////////////////////////////
//     Function: VirtualMouse::release_button
//       Access: Published
//  Description: Simulates the button being released.  This should
//               follow a previous call to press_button().
////////////////////////////////////////////////////////////////////
void VirtualMouse::
release_button(ButtonHandle button) {
  _next_button_events->push_back(ButtonEvent(button, false));
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualMouse::transmit_data
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void VirtualMouse::
transmit_data(AllTransitionsWrapper &data) {
  // Swap in the button events, and clear them for next time.
  _button_events->swap(_next_button_events);
  _next_button_events->clear();

  if (_mouse_on) {
    // The mouse is within the window.
    _pixel_xyz->set_value(LPoint3f(_mouse_x, _mouse_y, 0.0f));

    // Scale to range [-1,1]
    float xf = (2.0f * (float)_mouse_x) / (float)_win_width - 1.0f;
    float yf = 1.0f - (2.0f * (float)_mouse_y) / (float)_win_height;
    _xyz->set_value(LPoint3f(xf, yf, 0.0f));
    data = _got_mouse_attrib;

  } else {
    // The mouse isn't within the window.
    data = _no_mouse_attrib;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualMouse::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void VirtualMouse::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "VirtualMouse",
                DataNode::get_class_type());

  Vec3DataTransition::init_type();
  register_data_transition(_pixel_xyz_type, "PixelXYZ",
                           Vec3DataTransition::get_class_type());
  register_data_transition(_xyz_type, "XYZ",
                           Vec3DataTransition::get_class_type());
  ButtonEventDataTransition::init_type();
  register_data_transition(_button_events_type, "ButtonEvents",
                           ButtonEventDataTransition::get_class_type());
}
