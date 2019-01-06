/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseSubregion.h
 * @author drose
 * @date 2005-05-13
 */

#ifndef MOUSESUBREGION_H
#define MOUSESUBREGION_H

#include "pandabase.h"

#include "mouseInterfaceNode.h"
#include "luse.h"
#include "linmath_events.h"
#include "buttonEventList.h"


/**
 * The MouseSubregion object scales the mouse inputs from within a rectangular
 * region of the screen, as if they were the full-screen inputs.
 *
 * If you choose your MouseSubregion coordinates to exactly match a
 * DisplayRegion within your window, you end up with a virtual mouse within
 * your DisplayRegion.
 */
class EXPCL_PANDA_TFORM MouseSubregion : public MouseInterfaceNode {
PUBLISHED:
  explicit MouseSubregion(const std::string &name);
  ~MouseSubregion();

  INLINE PN_stdfloat get_left() const;
  INLINE PN_stdfloat get_right() const;
  INLINE PN_stdfloat get_bottom() const;
  INLINE PN_stdfloat get_top() const;
  INLINE void set_dimensions(PN_stdfloat l, PN_stdfloat r, PN_stdfloat b, PN_stdfloat t);

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  PN_stdfloat _l;
  PN_stdfloat _r;
  PN_stdfloat _b;
  PN_stdfloat _t;

  PN_stdfloat _minx, _miny;
  PN_stdfloat _scalex, _scaley;

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
