// Filename: graphicsWindowInputDevice.h
// Created by:  drose (24May00)
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

#ifndef GRAPHICSWINDOWINPUTDEVICE_H
#define GRAPHICSWINDOWINPUTDEVICE_H

#include "pandabase.h"

#include "buttonEvent.h"
#include "mouseData.h"

#include "pdeque.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : GraphicsWindowInputDevice
// Description : This is a structure representing a single input
//               device that may be associated with a window.
//               Typically this will be a keyboard/mouse pair, and
//               there will be exactly one of these associated with
//               each window, but other variants are possible.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsWindowInputDevice {
private:
  GraphicsWindowInputDevice(const string &name, int flags);

public:
  static GraphicsWindowInputDevice pointer_only(const string &name);
  static GraphicsWindowInputDevice keyboard_only(const string &name);
  static GraphicsWindowInputDevice pointer_and_keyboard(const string &name);

  INLINE GraphicsWindowInputDevice();
  GraphicsWindowInputDevice(const GraphicsWindowInputDevice &copy);
  void operator = (const GraphicsWindowInputDevice &copy);
  ~GraphicsWindowInputDevice();

  INLINE string get_name() const;
  INLINE bool has_pointer() const;
  INLINE bool has_keyboard() const;

  INLINE const MouseData &get_mouse_data() const;

  bool has_button_event() const;
  ButtonEvent get_button_event();

public:
  // The following interface is for the various kinds of
  // GraphicsWindows to record the data incoming on the device.
  void button_down(ButtonHandle button);
  void button_resume_down(ButtonHandle button);
  void button_up(ButtonHandle button);
  void keystroke(int keycode);
  INLINE void set_pointer_in_window(int x, int y);
  INLINE void set_pointer_out_of_window();

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

  string _name;
  int _flags;
  MouseData _mouse_data;
  ButtonEvents _button_events;
};

#include "graphicsWindowInputDevice.I"

#define EXPCL EXPCL_PANDA
#define EXPTP EXPTP_PANDA
#define TYPE GraphicsWindowInputDevice
#define NAME vector_GraphicsWindowInputDevice

#include "vector_src.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
