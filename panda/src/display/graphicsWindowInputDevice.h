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

#include "buttonEvent.h"
#include "pointerEvent.h"
#include "pointerEventList.h"
#include "mouseData.h"
#include "clockObject.h"

#include "pdeque.h"
#include "pvector.h"
#include "lightMutex.h"
#include "lightMutexHolder.h"

// Forward declarations
class GraphicsWindow;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsWindowInputDevice
// Description : This is a structure representing a single input
//               device that may be associated with a window.
//               Typically this will be a keyboard/mouse pair, and
//               there will be exactly one of these associated with
//               each window, but other variants are possible.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY GraphicsWindowInputDevice {
private:
  GraphicsWindowInputDevice(GraphicsWindow *host, const string &name, int flags);

public:
  static GraphicsWindowInputDevice pointer_only(GraphicsWindow *host, const string &name);
  static GraphicsWindowInputDevice keyboard_only(GraphicsWindow *host, const string &name);
  static GraphicsWindowInputDevice pointer_and_keyboard(GraphicsWindow *host, const string &name);

  INLINE GraphicsWindowInputDevice();
  GraphicsWindowInputDevice(const GraphicsWindowInputDevice &copy);
  void operator = (const GraphicsWindowInputDevice &copy);
  ~GraphicsWindowInputDevice();

  INLINE string get_name() const;
  INLINE bool has_pointer() const;
  INLINE bool has_keyboard() const;

  INLINE void set_device_index(int index);

  INLINE MouseData get_pointer() const;
  INLINE MouseData get_raw_pointer() const;

  INLINE void enable_pointer_events();
  INLINE void disable_pointer_events();

  void enable_pointer_mode(double speed);
  void disable_pointer_mode();

  bool has_button_event() const;
  ButtonEvent get_button_event();
  bool has_pointer_event() const;
  PT(PointerEventList) get_pointer_events();

PUBLISHED:
  // The following interface is for the various kinds of
  // GraphicsWindows to record the data incoming on the device.
  INLINE void button_down(ButtonHandle button);
  INLINE void button_resume_down(ButtonHandle button);
  INLINE void button_up(ButtonHandle button);
  INLINE void keystroke(int keycode);
  INLINE void focus_lost();
  INLINE void raw_button_down(ButtonHandle button);
  INLINE void raw_button_up(ButtonHandle button);
  INLINE void set_pointer_in_window(double x, double y);
  INLINE void set_pointer_out_of_window();

  void button_down(ButtonHandle button, double time);
  void button_resume_down(ButtonHandle button, double time);
  void button_up(ButtonHandle button, double time);
  void keystroke(int keycode, double time);
  void candidate(const wstring &candidate_string, size_t highlight_start,
                 size_t highlight_end, size_t cursor_pos);
  void focus_lost(double time);
  void raw_button_down(ButtonHandle button, double time);
  void raw_button_up(ButtonHandle button, double time);

  INLINE void set_pointer_in_window(double x, double y, double time);
  INLINE void set_pointer_out_of_window(double time);
  void set_pointer(bool inwin, double x, double y, double time);

public:
  // We need these methods to make VC++ happy when we try to
  // instantiate a pvector<GraphicsWindowInputDevice>.  They don't do
  // anything useful.
  INLINE bool operator == (const GraphicsWindowInputDevice &other) const;
  INLINE bool operator != (const GraphicsWindowInputDevice &other) const;
  INLINE bool operator < (const GraphicsWindowInputDevice &other) const;

private:
  enum InputDeviceFlags {
    IDF_has_pointer    = 0x01,
    IDF_has_keyboard   = 0x02
  };
  typedef pdeque<ButtonEvent> ButtonEvents;

  LightMutex _lock;

  GraphicsWindow *_host;

  string _name;
  int _flags;
  int _device_index;
  int _event_sequence;

  bool   _pointer_mode_enable;
  double _pointer_speed;

  bool _enable_pointer_events;
  MouseData _mouse_data;
  MouseData _true_mouse_data;
  ButtonEvents _button_events;
  PT(PointerEventList) _pointer_events;

  typedef pset<ButtonHandle> ButtonsHeld;
  ButtonsHeld _buttons_held;
};

#include "graphicsWindowInputDevice.I"

#define EXPCL EXPCL_PANDA_DISPLAY
#define EXPTP EXPTP_PANDA_DISPLAY
#define TYPE GraphicsWindowInputDevice
#define NAME vector_GraphicsWindowInputDevice

#include "vector_src.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
