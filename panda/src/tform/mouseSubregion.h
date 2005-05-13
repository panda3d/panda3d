// Filename: mouseSubregion.h
// Created by:  drose (13May05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef MOUSESUBREGION_H
#define MOUSESUBREGION_H

#include "pandabase.h"

#include "mouseInterfaceNode.h"
#include "luse.h"


////////////////////////////////////////////////////////////////////
//       Class : MouseSubregion
// Description : The MouseSubregion object scales the mouse inputs
//               from within a rectangular region of the screen, as if
//               they were the full-screen inputs.
//
//               If you choose your MouseSubregion coordinates to
//               exactly match a DisplayRegion within your window, you
//               end up with a virtual mouse within your
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MouseSubregion : public MouseInterfaceNode {
PUBLISHED:
  MouseSubregion(const string &name);
  ~MouseSubregion();

  INLINE float get_left() const;
  INLINE float get_right() const;
  INLINE float get_bottom() const;
  INLINE float get_top() const;
  INLINE void set_dimensions(float l, float r, float b, float t);

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  float _l;
  float _r;
  float _b;
  float _t;

  float _minx, _miny;
  float _scalex, _scaley;

private:
  // inputs
  int _pixel_xy_input;
  int _pixel_size_input;
  int _xy_input;
  int _button_events_input;

  // outputs
  int _pixel_xy_output;
  int _pixel_size_output;
  int _xy_output;
  int _button_events_output;

  PT(EventStoreVec2) _pixel_xy;
  PT(EventStoreVec2) _pixel_size;
  PT(EventStoreVec2) _xy;
  PT(ButtonEventList) _button_events;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MouseInterfaceNode::init_type();
    register_type(_type_handle, "MouseSubregion",
                  MouseInterfaceNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "mouseSubregion.I"

#endif
