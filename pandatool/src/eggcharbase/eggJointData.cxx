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
  _new_parent = (EggJointData *)NULL;
  _has_rest_frame = false;
  _rest_frames_differ = false;
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
//     Function: EggJointData::force_initial_rest_frame
//       Access: Public
//  Description: Forces all of the joints to have the same rest frame
//               value as the first joint read in.  This is a drastic
//               way to repair models whose rest frame values are
//               completely bogus, but should not be performed on
//               models that are otherwise correct.
////////////////////////////////////////////////////////////////////
void EggJointData::
force_initial_rest_frame() {
  if (!has_rest_frame()) {
    return;
  }
  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    if (has_model(model_index)) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, get_model(model_index));
      if (joint->is_of_type(EggJointNodePointer::get_class_type())) {
        joint->set_frame(0, get_rest_frame());
      }
    }
  }
  _rest_frames_differ = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::move_vertices_to
//       Access: Public
//  Description: Moves the vertices assigned to this joint into the
//               indicated joint, without changing their weight
//               assignments.
////////////////////////////////////////////////////////////////////
void EggJointData::
move_vertices_to(EggJointData *new_owner) {
  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    if (has_model(model_index) && new_owner->has_model(model_index)) {
      EggJointPointer *joint, *new_joint;
      DCAST_INTO_V(joint, get_model(model_index));
      DCAST_INTO_V(new_joint, new_owner->get_model(model_index));
      joint->move_vertices_to(new_joint);
    }
  }
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
//     Function: EggJointData::expose
//       Access: Public
//  Description: Calls expose() on all models for this joint, but does
//               not recurse downwards.
////////////////////////////////////////////////////////////////////
void EggJointData::
expose(EggGroup::DCSType dcs_type) {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != (EggBackPointer *)NULL) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);
      joint->expose(dcs_type);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::zero_channels
//       Access: Public
//  Description: Calls zero_channels() on all models for this joint,
//               but does not recurse downwards.
////////////////////////////////////////////////////////////////////
void EggJointData::
zero_channels(const string &components) {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != (EggBackPointer *)NULL) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);
      joint->zero_channels(components);
    }
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
  nassertv(egg_object != (EggObject *)NULL);
  if (egg_object->is_of_type(EggGroup::get_class_type())) {
    // It must be a <Joint>.
    EggJointNodePointer *joint = new EggJointNodePointer(egg_object);
    set_model(model_index, joint);
    if (!_has_rest_frame) {
      _rest_frame = joint->get_frame(0);
      _has_rest_frame = true;

    } else {
      // If this new node doesn't come within an acceptable tolerance
      // of our first reading of this joint's rest frame, set a
      // warning flag.
      if (!_rest_frame.almost_equal(joint->get_frame(0), 0.0001)) {
        _rest_frames_differ = true;
      }
    }

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

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::do_begin_reparent
//       Access: Protected
//  Description: Clears out the _children vector in preparation for
//               refilling it from the _new_parent information.
////////////////////////////////////////////////////////////////////
void EggJointData::
do_begin_reparent() {
  _children.clear();

  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    if (has_model(model_index)) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, get_model(model_index));
      joint->begin_rebuild();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::do_begin_compute_reparent
//       Access: Protected
//  Description: Eliminates any cached values before beginning a walk
//               through all the joints for do_compute_reparent(), for
//               a given model/frame.
////////////////////////////////////////////////////////////////////
void EggJointData::
do_begin_compute_reparent() { 
  _got_new_net_frame = false;
  _got_new_net_frame_inv = false;
  _computed_reparent = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::do_compute_reparent
//       Access: Protected
//  Description: Prepares the reparent operation by computing a new
//               transform for each frame of each model, designed to
//               keep the net transform the same when the joint is
//               moved to its new parent.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool EggJointData::
do_compute_reparent(int model_index, int n) {
  if (_computed_reparent) {
    // We've already done this joint.  This is possible because we
    // have to recursively compute joints upwards, so we might visit
    // the same joint more than once.
    return _computed_ok;
  }
  _computed_reparent = true;

  if (_parent == _new_parent) {
    // Trivial (and most common) case: we are not moving the joint.
    // No recomputation necessary.
    _computed_ok = true;
    return true;
  }

  EggBackPointer *back = get_model(model_index);
  if (back == (EggBackPointer *)NULL) {
    // This joint doesn't have any data to modify.
    _computed_ok = true;
    return true;
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, false);

  LMatrix4d transform;
  if (_parent == (EggJointData *)NULL) {
    // We are moving from outside the joint hierarchy to within it.
    transform = _new_parent->get_new_net_frame_inv(model_index, n);

  } else if (_new_parent == (EggJointData *)NULL) {
    // We are moving from within the hierarchy to outside it.
    transform = _parent->get_new_net_frame(model_index, n);

  } else {
    // We are changing parents within the hierarchy.
    transform = 
      _parent->get_new_net_frame(model_index, n) *
      _new_parent->get_new_net_frame_inv(model_index, n);
  }

  nassertr(n == joint->get_num_rebuild_frames(), false);

  _computed_ok = joint->add_rebuild_frame(joint->get_frame(n) * transform);
  return _computed_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::do_finish_reparent
//       Access: Protected
//  Description: Performs the actual reparenting operation
//               by removing all of the old children and replacing
//               them with the set of new children.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool EggJointData::
do_finish_reparent() {
  bool all_ok = true;

  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    EggJointPointer *parent_joint = NULL;
    if (_new_parent != NULL && _new_parent->has_model(model_index)) {
      DCAST_INTO_R(parent_joint, _new_parent->get_model(model_index), false);
    }

    if (has_model(model_index)) {
      EggJointPointer *joint;
      DCAST_INTO_R(joint, get_model(model_index), false);
      joint->do_finish_reparent(parent_joint);
      if (!joint->do_rebuild()) {
        all_ok = false;
      }
    }
  }

  _parent = _new_parent;
  if (_parent != (EggJointData *)NULL) {
    _parent->_children.push_back(this);
  }

  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::find_joint_exact
//       Access: Private
//  Description: The recursive implementation of find_joint, this
//               flavor searches recursively for an exact match of the
//               preferred joint name.
////////////////////////////////////////////////////////////////////
EggJointData *EggJointData::
find_joint_exact(const string &name) {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    if (child->get_name() == name) {
      return child;
    }
    EggJointData *result = child->find_joint_exact(name);
    if (result != (EggJointData *)NULL) {
      return result;
    }
  }

  return (EggJointData *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::find_joint_matches
//       Access: Private
//  Description: The recursive implementation of find_joint, this
//               flavor searches recursively for any acceptable match.
////////////////////////////////////////////////////////////////////
EggJointData *EggJointData::
find_joint_matches(const string &name) {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    if (child->matches_name(name)) {
      return child;
    }
    EggJointData *result = child->find_joint_matches(name);
    if (result != (EggJointData *)NULL) {
      return result;
    }
  }

  return (EggJointData *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_new_net_frame
//       Access: Private
//  Description: Similar to get_net_frame(), but computed for the
//               prospective new parentage of the node, before
//               do_finish_reparent() is called.  This is generally
//               useful only when called within do_compute_reparent().
////////////////////////////////////////////////////////////////////
const LMatrix4d &EggJointData::
get_new_net_frame(int model_index, int n) {
  if (!_got_new_net_frame) {
    _new_net_frame = get_new_frame(model_index, n);
    if (_new_parent != (EggJointData *)NULL) {
      _new_net_frame = _new_net_frame * _new_parent->get_new_net_frame(model_index, n);
    }
    _got_new_net_frame = true;
  }
  return _new_net_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_new_net_frame_inv
//       Access: Private
//  Description: Returns the inverse of get_new_net_frame().
////////////////////////////////////////////////////////////////////
const LMatrix4d &EggJointData::
get_new_net_frame_inv(int model_index, int n) {
  if (!_got_new_net_frame_inv) {
    _new_net_frame_inv.invert_from(get_new_frame(model_index, n));
    if (_new_parent != (EggJointData *)NULL) {
      _new_net_frame_inv = _new_parent->get_new_net_frame_inv(model_index, n) * _new_net_frame_inv;
    }
    _got_new_net_frame_inv = true;
  }
  return _new_net_frame_inv;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::get_new_frame
//       Access: Private
//  Description: Returns the local transform matrix corresponding to
//               this joint position in the nth frame in the indicated
//               model, as it will be when do_finish_reparent() is
//               called.
////////////////////////////////////////////////////////////////////
LMatrix4d EggJointData::
get_new_frame(int model_index, int n) {
  do_compute_reparent(model_index, n);

  EggBackPointer *back = get_model(model_index);
  if (back == (EggBackPointer *)NULL) {
    return LMatrix4d::ident_mat();
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, LMatrix4d::ident_mat());

  if (joint->get_num_rebuild_frames() > 0) {
    return joint->get_rebuild_frame(n);
  } else {
    return joint->get_frame(n);
  }
}
