// Filename: qpvirtualMouse.h
// Created by:  drose (12Mar02)
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

#ifndef qpVIRTUALMOUSE_H
#define qpVIRTUALMOUSE_H

#include "pandabase.h"

#include "qpdataNode.h"
#include "buttonHandle.h"
#include "buttonEvent.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : qpVirtualMouse
// Description : Poses as a MouseAndKeyboard object in the datagraph,
//               but accepts input from user calls, rather than
//               reading the actual mouse and keyboard from an input
//               device.  The user can write high-level code to put
//               the mouse wherever he/she wants, and to insert
//               keypresses on demand.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpVirtualMouse : public qpDataNode {
PUBLISHED:
  qpVirtualMouse(const string &name);

  void set_mouse_pos(int x, int y);
  void set_window_size(int width, int height);
  void set_mouse_on(bool flag);
  
  void press_button(ButtonHandle button);
  void release_button(ButtonHandle button);

private:
  int _mouse_x, _mouse_y;
  int _win_width, _win_height;
  bool _mouse_on;

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // outputs
  int _pixel_xy_output;
  int _xy_output;
  int _button_events_output;

  PT(EventStoreVec2) _pixel_xy;
  PT(EventStoreVec2) _xy;
  PT(ButtonEventList) _button_events;
  PT(ButtonEventList) _next_button_events;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpDataNode::init_type();
    register_type(_type_handle, "qpMouseAndKeyboard",
                  qpDataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif

