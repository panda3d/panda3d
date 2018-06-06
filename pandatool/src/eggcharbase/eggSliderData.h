/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSliderData.h
 * @author drose
 * @date 2001-02-26
 */

#ifndef EGGSLIDERDATA_H
#define EGGSLIDERDATA_H

#include "pandatoolbase.h"

#include "eggComponentData.h"


/**
 * This corresponds to a single morph slider control.  It contains back
 * pointers to all the vertices and primitives that reference this slider
 * across all models, as well as all the tables in which it appears in all
 * animation files.
 */
class EggSliderData : public EggComponentData {
public:
  EggSliderData(EggCharacterCollection *collection,
                EggCharacterData *char_data);

  double get_frame(int model_index, int n) const;

  virtual void add_back_pointer(int model_index, EggObject *egg_object);
  virtual void write(std::ostream &out, int indent_level = 0) const;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggComponentData::init_type();
    register_type(_type_handle, "EggSliderData",
                  EggComponentData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggSliderData.I"

#endif
