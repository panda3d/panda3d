// Filename: mouseAndKeyboard.h
// Created by:  drose (12Mar02)
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

#ifndef MOUSEANDKEYBOARD_H
#define MOUSEANDKEYBOARD_H

#include "pandabase.h"

#include "dataNode.h"
#include "buttonEventList.h"
#include "pointerEventList.h"
#include "linmath_events.h"
#include "pointerTo.h"
#include "graphicsWindow.h"

////////////////////////////////////////////////////////////////////
//       Class : MouseAndKeyboard
// Description : Reads the mouse and/or keyboard data sent from a
//               GraphicsWindow, and transmits it down the data graph.
//
//               The mouse and keyboard devices are bundled together
//               into one device here, because they interrelate so
//               much.  A mouse might be constrained by the holding
//               down of the shift key, for instance, or the clicking
//               of the mouse button might be handled in much the same
//               way as a keyboard key.
//
//               Mouse data is sent down the data graph as an x,y
//               position as well as the set of buttons currently
//               being held down; keyboard data is sent down as a set
//               of keypress events in an EventDataTransition.  To
//               throw these events to the system, you must attach an
//               EventThrower to the MouseAndKeyboard object;
//               otherwise, the events will be discarded.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DEVICE MouseAndKeyboard : public DataNode {
PUBLISHED:
  MouseAndKeyboard(GraphicsWindow *window, int device, const string &name);
  void set_source(GraphicsWindow *window, int device);

  PT(GraphicsWindow) get_source_window() const;
  int                get_source_device() const;
  
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
  int _pointer_events_output;

  PT(EventStoreVec2) _pixel_xy;
  PT(EventStoreVec2) _pixel_size;
  PT(EventStoreVec2) _xy;
  PT(ButtonEventList) _button_events;

  PT(GraphicsWindow) _window;
  int _device;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "MouseAndKeyboard",
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
