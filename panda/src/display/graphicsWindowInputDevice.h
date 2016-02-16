// Filename: graphicsWindowInputDevice.h
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

#ifndef GRAPHICSWINDOWINPUTDEVICE_H
#define GRAPHICSWINDOWINPUTDEVICE_H

#include "pandabase.h"
#include "inputDevice.h"

// Forward declarations
class GraphicsWindow;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsWindowInputDevice
// Description : This is a virtual input device that represents
//               the keyboard and mouse pair that is associated with
//               a particular window.  It collects mouse and keyboard
//               events from the windowing system while the window is
//               in focus.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY GraphicsWindowInputDevice : public InputDevice {
private:
  GraphicsWindowInputDevice(GraphicsWindow *host, const string &name, int flags);

public:
  static PT(GraphicsWindowInputDevice) pointer_only(GraphicsWindow *host, const string &name);
  static PT(GraphicsWindowInputDevice) keyboard_only(GraphicsWindow *host, const string &name);
  static PT(GraphicsWindowInputDevice) pointer_and_keyboard(GraphicsWindow *host, const string &name);

  INLINE GraphicsWindowInputDevice();
  GraphicsWindowInputDevice(const GraphicsWindowInputDevice &copy);
  void operator = (const GraphicsWindowInputDevice &copy);
  ~GraphicsWindowInputDevice();

  INLINE void set_device_index(int index);

PUBLISHED:
  // The following interface is for the various kinds of
  // GraphicsWindows to record the data incoming on the device.
  void button_down(ButtonHandle button, double time = ClockObject::get_global_clock()->get_frame_time());
  void button_resume_down(ButtonHandle button, double time = ClockObject::get_global_clock()->get_frame_time());
  void button_up(ButtonHandle button, double time = ClockObject::get_global_clock()->get_frame_time());

  void keystroke(int keycode, double time = ClockObject::get_global_clock()->get_frame_time());
  void candidate(const wstring &candidate_string, size_t highlight_start,
                 size_t highlight_end, size_t cursor_pos);

  void focus_lost(double time = ClockObject::get_global_clock()->get_frame_time());

  void raw_button_down(ButtonHandle button, double time = ClockObject::get_global_clock()->get_frame_time());
  void raw_button_up(ButtonHandle button, double time = ClockObject::get_global_clock()->get_frame_time());

  INLINE void set_pointer_in_window(double x, double y, double time = ClockObject::get_global_clock()->get_frame_time());
  INLINE void set_pointer_out_of_window(double time = ClockObject::get_global_clock()->get_frame_time());

private:
  GraphicsWindow *_host;
  int _device_index;

  typedef pset<ButtonHandle> ButtonsHeld;
  ButtonsHeld _buttons_held;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    InputDevice::init_type();
    register_type(_type_handle, "GraphicsWindowInputDevice",
                  InputDevice::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "graphicsWindowInputDevice.I"

#endif
