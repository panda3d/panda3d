// Filename: eggJointData.cxx
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggJointData.h"
#include "eggJointPointer.h"
#include "eggMatrixTablePointer.h"

#include <eggGroup.h>
#include <eggTable.h>
#include <indent.h>


////////////////////////////////////////////////////////////////////
//     Function: EggJointData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggJointData::
EggJointData(EggCharacterCollection *collection, 
	     EggCharacterData *char_data) :
  EggComponentData(collection, char_data)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::add_back_pointer
//       Access: Public, Virtual
//  Description: Adds the indicated model joint or anim table to the
//               data.
////////////////////////////////////////////////////////////////////
void EggJointData::
add_back_pointer(int model_index, EggObject *egg_object) {
  if (egg_object->is_of_type(EggGroup::get_class_type())) {
    // It must be a <Joint>.
    EggJointPointer *joint = new EggJointPointer(egg_object);
    set_back_pointer(model_index, joint);

  } else if (egg_object->is_of_type(EggTable::get_class_type())) {
    // It's a <Table> with an "xform" child beneath it.
    EggMatrixTablePointer *xform = new EggMatrixTablePointer(egg_object);
    set_back_pointer(model_index, xform);

  } else {
    nout << "Invalid object added to joint for back pointer.\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void EggJointData::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "Joint " << get_name()
    << " (models:";
  int num_back_pointers = get_num_back_pointers();
  for (int model_index = 0; model_index < num_back_pointers; model_index++) {
    if (has_back_pointer(model_index)) {
      out << " " << model_index;
    }
  }
  out << ") {\n";

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write(out, indent_level + 2);
  }

  indent(out, indent_level) << "}\n";
}
