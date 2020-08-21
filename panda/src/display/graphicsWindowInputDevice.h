/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsWindowInputDevice.h
 * @author drose
 * @date 2000-05-24
 */

#ifndef GRAPHICSWINDOWINPUTDEVICE_H
#define GRAPHICSWINDOWINPUTDEVICE_H

#include "pandabase.h"
#include "inputDevice.h"

// Forward declarations
class GraphicsWindow;

/**
 * This is a virtual input device that represents the keyboard and mouse pair
 * that is associated with a particular window.  It collects mouse and
 * keyboard events from the windowing system while the window is in focus.
 */
class EXPCL_PANDA_DISPLAY GraphicsWindowInputDevice : public InputDevice {
private:
  GraphicsWindowInputDevice(GraphicsWindow *host, const std::string &name,
                            bool pointer, bool keyboard);

public:
  static PT(GraphicsWindowInputDevice) pointer_only(GraphicsWindow *host, const std::string &name);
  static PT(GraphicsWindowInputDevice) keyboard_only(GraphicsWindow *host, const std::string &name);
  static PT(GraphicsWindowInputDevice) pointer_and_keyboard(GraphicsWindow *host, const std::string &name);

  GraphicsWindowInputDevice() = default;

PUBLISHED:
  // The following interface is for the various kinds of GraphicsWindows to
  // record the data incoming on the device.
  void button_down(ButtonHandle button, double time = ClockObject::get_global_clock()->get_frame_time());
  void button_resume_down(ButtonHandle button, double time = ClockObject::get_global_clock()->get_frame_time());
  void button_up(ButtonHandle button, double time = ClockObject::get_global_clock()->get_frame_time());

  void keystroke(int keycode, double time = ClockObject::get_global_clock()->get_frame_time());
  void candidate(const std::wstring &candidate_string, size_t highlight_start,
                 size_t highlight_end, size_t cursor_pos);

  void focus_lost(double time = ClockObject::get_global_clock()->get_frame_time());

  void raw_button_down(ButtonHandle button, double time = ClockObject::get_global_clock()->get_frame_time());
  void raw_button_up(ButtonHandle button, double time = ClockObject::get_global_clock()->get_frame_time());

  INLINE PointerData get_pointer() const;
  void set_pointer_in_window(double x, double y, double time = ClockObject::get_global_clock()->get_frame_time());
  void set_pointer_out_of_window(double time = ClockObject::get_global_clock()->get_frame_time());
  INLINE void update_pointer(PointerData data, double time = ClockObject::get_global_clock()->get_frame_time());
  INLINE void pointer_moved(double x, double y, double time = ClockObject::get_global_clock()->get_frame_time());
  INLINE void remove_pointer(int id);

private:
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
