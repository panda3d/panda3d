// Filename: eggSliderData.cxx
// Created by:  drose (26Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggSliderData.h"
#include "eggVertexPointer.h"

#include <eggPrimitive.h>
#include <eggVertex.h>
#include <eggSAnimData.h>
#include <indent.h>

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
      back = new EggVertexPointer;
      set_model(model_index, back);
    }

  } else if (egg_object->is_of_type(EggVertex::get_class_type())) {
    // A vertex!
    EggBackPointer *back = get_model(model_index);
    if (back == (EggBackPointer *)NULL) {
      back = new EggVertexPointer;
      set_model(model_index, back);
    }

  } else if (egg_object->is_of_type(EggSAnimData::get_class_type())) {
    // A slider animation table!  Woo hoo!
    EggBackPointer *back = get_model(model_index);
    if (back == (EggBackPointer *)NULL) {
      back = new EggVertexPointer;
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
