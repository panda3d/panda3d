/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseTrackball.h
 * @author D. Lawrence
 * @date 2019-08-17
 */

#ifndef MOUSE_TRACKBALL_H
#define MOUSE_TRACKBALL_H

#include "trackball.h"

class EXPCL_PANDA_TFORM MouseTrackball : public Trackball {
PUBLISHED:
  explicit MouseTrackball(const std::string &name);

private:
  int _last_button;
  PN_stdfloat _lastx, _lasty;

  // inputs
  int _pixel_xy_input;

  void apply(double x, double y, int button);

protected:
  // Inherited from DataNode
  void do_transmit_data(DataGraphTraverser *trav,
                        const DataNodeTransmit &input,
                        DataNodeTransmit &output) override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Trackball::init_type();
    register_type(_type_handle, "MouseTrackball",
                  Trackball::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif