/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSliderData.cxx
 * @author drose
 * @date 2001-02-26
 */

#include "eggSliderData.h"
#include "eggVertexPointer.h"
#include "eggScalarTablePointer.h"
#include "eggSliderPointer.h"
#include "dcast.h"
#include "eggPrimitive.h"
#include "eggVertex.h"
#include "eggSAnimData.h"
#include "indent.h"

TypeHandle EggSliderData::_type_handle;

/**
 *
 */
EggSliderData::
EggSliderData(EggCharacterCollection *collection,
              EggCharacterData *char_data) :
  EggComponentData(collection, char_data)
{
}

/**
 * Returns the value corresponding to this slider position in the nth frame in
 * the indicated model.
 */
double EggSliderData::
get_frame(int model_index, int n) const {
  EggBackPointer *back = get_model(model_index);
  if (back == nullptr) {
    return 0.0;
  }

  EggSliderPointer *slider;
  DCAST_INTO_R(slider, back, 0.0);

  return slider->get_frame(n);
}

/**
 * Adds the indicated vertex, primitive, or morph table to the data.
 */
void EggSliderData::
add_back_pointer(int model_index, EggObject *egg_object) {
  if (egg_object->is_of_type(EggPrimitive::get_class_type())) {
    // A primitive!
    EggBackPointer *back = get_model(model_index);
    if (back == nullptr) {
      back = new EggVertexPointer(egg_object);
      set_model(model_index, back);
    }

  } else if (egg_object->is_of_type(EggVertex::get_class_type())) {
    // A vertex!
    EggBackPointer *back = get_model(model_index);
    if (back == nullptr) {
      back = new EggVertexPointer(egg_object);
      set_model(model_index, back);
    }

  } else if (egg_object->is_of_type(EggSAnimData::get_class_type())) {
    // A slider animation table!  Woo hoo!
    EggBackPointer *back = get_model(model_index);
    if (back == nullptr) {
      back = new EggScalarTablePointer(egg_object);
      set_model(model_index, back);
    }

  } else {
    nout << "Invalid object added to slider for back pointer.\n";
  }
}

/**
 *
 */
void EggSliderData::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "Slider " << get_name()
    << " (models:";
  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    if (has_model(model_index)) {
      out << " " << model_index;
    }
  }
  out << ")\n";
}
