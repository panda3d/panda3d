// Filename: eggSliderData.cxx
// Created by:  drose (26Feb01)
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

#include "eggSliderData.h"
#include "eggVertexPointer.h"
#include "eggScalarTablePointer.h"
#include "eggSliderPointer.h"

#include "eggPrimitive.h"
#include "eggVertex.h"
#include "eggSAnimData.h"
#include "indent.h"

TypeHandle EggSliderData::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggSliderData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggSliderData::
EggSliderData(EggCharacterCollection *collection,
              EggCharacterData *char_data) :
  EggComponentData(collection, char_data)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggSliderData::get_num_frames
//       Access: Public
//  Description: Returns the number of frames of animation for this
//               particular slider in the indicated model.
////////////////////////////////////////////////////////////////////
int EggSliderData::
get_num_frames(int model_index) const {
  EggBackPointer *back = get_model(model_index);
  if (back == (EggBackPointer *)NULL) {
    return 0;
  }

  EggSliderPointer *slider;
  DCAST_INTO_R(slider, back, 0);

  return slider->get_num_frames();
}

////////////////////////////////////////////////////////////////////
//     Function: EggSliderData::get_frame
//       Access: Public
//  Description: Returns the value corresponding to this slider
//               position in the nth frame in the indicated model.
////////////////////////////////////////////////////////////////////
double EggSliderData::
get_frame(int model_index, int n) const {
  EggBackPointer *back = get_model(model_index);
  if (back == (EggBackPointer *)NULL) {
    return 0.0;
  }

  EggSliderPointer *slider;
  DCAST_INTO_R(slider, back, 0.0);

  return slider->get_frame(n);
}

////////////////////////////////////////////////////////////////////
//     Function: EggSliderData::add_back_pointer
//       Access: Public, Virtual
//  Description: Adds the indicated vertex, primitive, or morph table
//               to the data.
////////////////////////////////////////////////////////////////////
void EggSliderData::
add_back_pointer(int model_index, EggObject *egg_object) {
  if (egg_object->is_of_type(EggPrimitive::get_class_type())) {
    // A primitive!
    EggBackPointer *back = get_model(model_index);
    if (back == (EggBackPointer *)NULL) {
      back = new EggVertexPointer(egg_object);
      set_model(model_index, back);
    }

  } else if (egg_object->is_of_type(EggVertex::get_class_type())) {
    // A vertex!
    EggBackPointer *back = get_model(model_index);
    if (back == (EggBackPointer *)NULL) {
      back = new EggVertexPointer(egg_object);
      set_model(model_index, back);
    }

  } else if (egg_object->is_of_type(EggSAnimData::get_class_type())) {
    // A slider animation table!  Woo hoo!
    EggBackPointer *back = get_model(model_index);
    if (back == (EggBackPointer *)NULL) {
      back = new EggScalarTablePointer(egg_object);
      set_model(model_index, back);
    }

  } else {
    nout << "Invalid object added to slider for back pointer.\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggSliderData::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void EggSliderData::
write(ostream &out, int indent_level) const {
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
