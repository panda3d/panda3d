/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSliderPointer.h
 * @author drose
 * @date 2003-07-18
 */

#ifndef EGGSLIDERPOINTER_H
#define EGGSLIDERPOINTER_H

#include "pandatoolbase.h"

#include "eggBackPointer.h"

#include "luse.h"

/**
 * This is a base class for EggVertexPointer and EggScalarTablePointer.
 */
class EggSliderPointer : public EggBackPointer {
public:
  virtual int get_num_frames() const=0;
  virtual double get_frame(int n) const=0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggBackPointer::init_type();
    register_type(_type_handle, "EggSliderPointer",
                  EggBackPointer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
