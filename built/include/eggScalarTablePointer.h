/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggScalarTablePointer.h
 * @author drose
 * @date 2003-07-18
 */

#ifndef EGGSCALARTABLEPOINTER_H
#define EGGSCALARTABLEPOINTER_H

#include "pandatoolbase.h"

#include "eggSliderPointer.h"

#include "eggSAnimData.h"
#include "pointerTo.h"

/**
 * This stores a pointer back to an EggSAnimData table (i.e.  an <S$Anim>
 * entry in an egg file), corresponding to the animation data from a single
 * bundle for this slider.
 */
class EggScalarTablePointer : public EggSliderPointer {
public:
  EggScalarTablePointer(EggObject *object);

  virtual double get_frame_rate() const;
  virtual int get_num_frames() const;
  virtual void extend_to(int num_frames);
  virtual double get_frame(int n) const;

  virtual void set_name(const std::string &name);

private:
  PT(EggSAnimData) _data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggSliderPointer::init_type();
    register_type(_type_handle, "EggScalarTablePointer",
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
