// Filename: mouse.h
// Created by:  mike (09Jan97)
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
#ifndef MOUSE_H
#define MOUSE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "dataNode.h"
#include "vec3DataTransition.h"
#include "buttonEventDataTransition.h"
#include "graphicsWindow.h"
#include "pointerTo.h"
#include "allTransitionsWrapper.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
const int MIN_MOVE = 2;

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
//               throw these events to the system, you must child an
//               EventThrower to the MouseAndKeyboard object;
//               otherwise, the events will be discarded.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MouseAndKeyboard : public DataNode {
PUBLISHED:
  MouseAndKeyboard(GraphicsWindow *window, int device,
                   const string& name = "");

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

protected:
  PT(GraphicsWindow) _window;
  int _device;

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
