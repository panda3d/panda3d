// Filename: virtualMouse.h
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

#ifndef VIRTUALMOUSE_H
#define VIRTUALMOUSE_H

#include "pandabase.h"

#include "dataNode.h"
#include "buttonHandle.h"
#include "buttonEvent.h"
#include "vec3DataTransition.h"
#include "buttonEventDataTransition.h"
#include "pointerTo.h"
#include "allTransitionsWrapper.h"

////////////////////////////////////////////////////////////////////
//       Class : VirtualMouse
// Description : Poses as a MouseAndKeyboard object in the datagraph,
//               but accepts input from user calls, rather than
//               reading the actual mouse and keyboard from an input
//               device.  The user can write high-level code to put
//               the mouse wherever he/she wants, and to insert
//               keypresses on demand.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VirtualMouse : public DataNode {
PUBLISHED:
  VirtualMouse(const string &name = "");

  void set_mouse_pos(int x, int y);
  void set_window_size(int width, int height);
  void set_mouse_on(bool flag);
  
  void press_button(ButtonHandle button);
  void release_button(ButtonHandle button);

public:
  virtual void transmit_data(AllTransitionsWrapper &data);

public:
  AllTransitionsWrapper _got_mouse_attrib;
  AllTransitionsWrapper _no_mouse_attrib;
  PT(Vec3DataTransition) _pixel_xyz;
  PT(Vec3DataTransition) _xyz;
  PT(ButtonEventDataTransition) _button_events;

  static TypeHandle _mods_type;
  static TypeHandle _pixel_xyz_type;
  static TypeHandle _xyz_type;
  static TypeHandle _button_events_type;

private:
  int _mouse_x, _mouse_y;
  int _win_width, _win_height;
  bool _mouse_on;
  PT(ButtonEventDataTransition) _next_button_events;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#endif
