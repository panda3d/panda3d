// Filename: eggJointData.cxx
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggJointData.h"
#include "eggJointNodePointer.h"
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
//     Function: EggJointData::find_joint
//       Access: Public
//  Description: Returns the first descendent joint found with the
//               indicated name, or NULL if no joint has that name.
////////////////////////////////////////////////////////////////////
INLINE EggJointData *EggJointData::
find_joint(const string &name) {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    if (child->get_name() == name) {
      return child;
    }
    EggJointData *result = child->find_joint(name);
    if (result != (EggJointData *)NULL) {
      return result;
    }
  }

  return (EggJointData *)NULL;
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
    EggJointNodePointer *joint = new EggJointNodePointer(egg_object);
    set_model(model_index, joint);

  } else if (egg_object->is_of_type(EggTable::get_class_type())) {
    // It's a <Table> with an "xform" child beneath it.
    EggMatrixTablePointer *xform = new EggMatrixTablePointer(egg_object);
    set_model(model_index, xform);

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
  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    if (has_model(model_index)) {
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
