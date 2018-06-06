/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualMouse.h
 * @author drose
 * @date 2002-03-12
 */

#ifndef VIRTUALMOUSE_H
#define VIRTUALMOUSE_H

#include "pandabase.h"
#include "dataNode.h"
#include "buttonHandle.h"
#include "buttonEvent.h"
#include "pointerTo.h"
#include "luse.h"
#include "linmath_events.h"
#include "buttonEventList.h"

/**
 * Poses as a MouseAndKeyboard object in the datagraph, but accepts input from
 * user calls, rather than reading the actual mouse and keyboard from an input
 * device.  The user can write high-level code to put the mouse wherever
 * he/she wants, and to insert keypresses on demand.
 */
class EXPCL_PANDA_DEVICE VirtualMouse : public DataNode {
PUBLISHED:
  explicit VirtualMouse(const std::string &name);

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
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // outputs
  int _pixel_xy_output;
  int _pixel_size_output;
  int _xy_output;
  int _button_events_output;

  PT(EventStoreVec2) _pixel_xy;
  PT(EventStoreVec2) _pixel_size;
  PT(EventStoreVec2) _xy;
  PT(ButtonEventList) _button_events;
  PT(ButtonEventList) _next_button_events;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "VirtualMouse",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
