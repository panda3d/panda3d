/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggVertexPointer.h
 * @author drose
 * @date 2001-02-26
 */

#ifndef EGGVERTEXPOINTER_H
#define EGGVERTEXPOINTER_H

#include "pandatoolbase.h"

#include "eggSliderPointer.h"

#include "eggGroup.h"
#include "pointerTo.h"

/**
 * This stores a pointer back to a <Vertex>, or to a particular pritimive like
 * a <Polygon>, representing a morph offset.
 */
class EggVertexPointer : public EggSliderPointer {
public:
  EggVertexPointer(EggObject *egg_object);

  virtual int get_num_frames() const;
  virtual double get_frame(int n) const;

  virtual bool has_vertices() const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggSliderPointer::init_type();
    register_type(_type_handle, "EggVertexPointer",
                  EggSliderPointer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
