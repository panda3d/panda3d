// Filename: eggJointData.cxx
// Created by:  drose (23Feb01)
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

#include "eggJointData.h"
#include "eggJointNodePointer.h"
#include "eggMatrixTablePointer.h"

#include "dcast.h"
#include "eggGroup.h"
#include "eggTable.h"
#include "indent.h"

TypeHandle EggJointData::_type_handle;


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
  _parent = (EggJointData *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::find_joint
//       Access: Public
//  Description: Returns the first descendent joint found with the
//               indicated name, or NULL if no joint has that name.
////////////////////////////////////////////////////////////////////
EggJointData *EggJointData::
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
//     Function: EggJointData::get_num_frames
//       Access: Public
//  Description: Returns the number of frames of animation for this
//               particular joint in the indicated model.
////////////////////////////////////////////////////////////////////
int EggJointData::
get_num_frames(int model_index) const {
  EggBackPointer *back = get_model(model_index);
  if (back == (EggBackPointer *)NULL) {
    return 0;
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, 0);

  return joint->get_num_frames();
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_frame
//       Access: Public
//  Description: Returns the local transform matrix corresponding to
//               this joint position in the nth frame in the indicated
//               model.
////////////////////////////////////////////////////////////////////
LMatrix4d EggJointData::
get_frame(int model_index, int n) const {
  EggBackPointer *back = get_model(model_index);
  if (back == (EggBackPointer *)NULL) {
    return LMatrix4d::ident_mat();
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, LMatrix4d::ident_mat());

  return joint->get_frame(n);
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_net_frame
//       Access: Public
//  Description: Returns the complete transform from the root
//               corresponding to this joint position in the nth frame
//               in the indicated model.
////////////////////////////////////////////////////////////////////
LMatrix4d EggJointData::
get_net_frame(int model_index, int n) const {
  LMatrix4d mat = get_frame(model_index, n);
  if (_parent != (EggJointData *)NULL) {
    mat = mat * _parent->get_net_frame(model_index, n);
  }
  return mat;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::do_rebuild
//       Access: Public
//  Description: Calls do_rebuild() on all models, and recursively on
//               all joints at this node and below.  Returns true if
//               all models returned true, false otherwise.
////////////////////////////////////////////////////////////////////
bool EggJointData::
do_rebuild() {
  bool all_ok = true;

  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != (EggBackPointer *)NULL) {
      EggJointPointer *joint;
      DCAST_INTO_R(joint, back, false);
      if (!joint->do_rebuild()) {
        all_ok = false;
      }
    }
  }

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    if (!child->do_rebuild()) {
      all_ok = false;
    }
  }

  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::optimize
//       Access: Public
//  Description: Calls optimize() on all models, and recursively on
//               all joints at this node and below.
////////////////////////////////////////////////////////////////////
void EggJointData::
optimize() {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != (EggBackPointer *)NULL) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);
      joint->optimize();
    }
  }

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    child->optimize();
  }
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
