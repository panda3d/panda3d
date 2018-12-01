/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggJointData.cxx
 * @author drose
 * @date 2001-02-23
 */

#include "eggJointData.h"

#include "eggCharacterDb.h"
#include "eggJointNodePointer.h"
#include "eggMatrixTablePointer.h"
#include "pvector.h"
#include "dcast.h"
#include "eggGroup.h"
#include "eggTable.h"
#include "indent.h"
#include "fftCompressor.h"
#include "zStream.h"

using std::string;

TypeHandle EggJointData::_type_handle;


/**
 *
 */
EggJointData::
EggJointData(EggCharacterCollection *collection,
             EggCharacterData *char_data) :
  EggComponentData(collection, char_data)
{
  _parent = nullptr;
  _new_parent = nullptr;
  _has_rest_frame = false;
  _rest_frames_differ = false;
}

/**
 * Returns the local transform matrix corresponding to this joint position in
 * the nth frame in the indicated model.
 */
LMatrix4d EggJointData::
get_frame(int model_index, int n) const {
  EggBackPointer *back = get_model(model_index);
  if (back == nullptr) {
    return LMatrix4d::ident_mat();
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, LMatrix4d::ident_mat());

  return joint->get_frame(n);
}

/**
 * Returns the complete transform from the root corresponding to this joint
 * position in the nth frame in the indicated model.
 */
LMatrix4d EggJointData::
get_net_frame(int model_index, int n, EggCharacterDb &db) const {
  EggBackPointer *back = get_model(model_index);
  if (back == nullptr) {
    return LMatrix4d::ident_mat();
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, LMatrix4d::ident_mat());

  LMatrix4d mat;
  if (!db.get_matrix(joint, EggCharacterDb::TT_net_frame, n, mat)) {
    // Compute this frame's net, and stuff it in.
    mat = get_frame(model_index, n);
    if (_parent != nullptr) {
      mat = mat * _parent->get_net_frame(model_index, n, db);
    }
    db.set_matrix(joint, EggCharacterDb::TT_net_frame, n, mat);
  }

  return mat;
}

/**
 * Returns the inverse of get_net_frame().
 */
LMatrix4d EggJointData::
get_net_frame_inv(int model_index, int n, EggCharacterDb &db) const {
  EggBackPointer *back = get_model(model_index);
  if (back == nullptr) {
    return LMatrix4d::ident_mat();
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, LMatrix4d::ident_mat());

  LMatrix4d mat;
  if (!db.get_matrix(joint, EggCharacterDb::TT_net_frame_inv, n, mat)) {
    // Compute this frame's net inverse, and stuff it in.
    LMatrix4d mat = get_net_frame(model_index, n, db);
    mat.invert_in_place();
    db.set_matrix(joint, EggCharacterDb::TT_net_frame_inv, n, mat);
  }

  return mat;
}

/**
 * Forces all of the joints to have the same rest frame value as the first
 * joint read in.  This is a drastic way to repair models whose rest frame
 * values are completely bogus, but should not be performed on models that are
 * otherwise correct.
 */
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

/**
 * Moves the vertices assigned to this joint into the indicated joint, without
 * changing their weight assignments.
 */
void EggJointData::
move_vertices_to(EggJointData *new_owner) {
  int num_models = get_num_models();

  if (new_owner == nullptr) {
    for (int model_index = 0; model_index < num_models; model_index++) {
      if (has_model(model_index)) {
        EggJointPointer *joint;
        DCAST_INTO_V(joint, get_model(model_index));
        joint->move_vertices_to(nullptr);
      }
    }
  } else {
    for (int model_index = 0; model_index < num_models; model_index++) {
      if (has_model(model_index) && new_owner->has_model(model_index)) {
        EggJointPointer *joint, *new_joint;
        DCAST_INTO_V(joint, get_model(model_index));
        DCAST_INTO_V(new_joint, new_owner->get_model(model_index));
        joint->move_vertices_to(new_joint);
      }
    }
  }
}

/**
 * Computes a score >= 0 reflecting the similarity of the current joint's
 * animation (in world space) to that of the indicated potential parent joint
 * (in world space).  The lower the number, the more similar the motion, and
 * the more suitable is the proposed parent-child relationship.  Returns -1 if
 * there is an error.
 */
int EggJointData::
score_reparent_to(EggJointData *new_parent, EggCharacterDb &db) {
  if (!FFTCompressor::is_compression_available()) {
    // If we don't have compression compiled in, we can't meaningfully score
    // the joints.
    return -1;
  }

  // First, build up a big array of the new transforms this joint would
  // receive in all frames of all models, were it reparented to the indicated
  // joint.
  vector_stdfloat i, j, k, a, b, c, x, y, z;
  pvector<LVecBase3> hprs;
  int num_rows = 0;

  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    EggBackPointer *back = get_model(model_index);
    if (back != nullptr) {
      EggJointPointer *joint;
      DCAST_INTO_R(joint, back, false);

      int num_frames = get_num_frames(model_index);
      for (int n = 0; n < num_frames; n++) {
        LMatrix4d transform;
        if (_parent == new_parent) {
          // We already have this parent.
          transform = LMatrix4d::ident_mat();

        } else if (_parent == nullptr) {
          // We are moving from outside the joint hierarchy to within it.
          transform = new_parent->get_net_frame_inv(model_index, n, db);

        } else if (new_parent == nullptr) {
          // We are moving from within the hierarchy to outside it.
          transform = _parent->get_net_frame(model_index, n, db);

        } else {
          // We are changing parents within the hierarchy.
          transform =
            _parent->get_net_frame(model_index, n, db) *
            new_parent->get_net_frame_inv(model_index, n, db);
        }

        transform = joint->get_frame(n) * transform;
        LVecBase3d scale, shear, hpr, translate;
        if (!decompose_matrix(transform, scale, shear, hpr, translate)) {
          // Invalid transform.
          return -1;
        }
        i.push_back(scale[0]);
        j.push_back(scale[1]);
        k.push_back(scale[2]);
        a.push_back(shear[0]);
        b.push_back(shear[1]);
        c.push_back(shear[2]);
        hprs.push_back(LCAST(PN_stdfloat, hpr));
        x.push_back(translate[0]);
        y.push_back(translate[1]);
        z.push_back(translate[2]);
        num_rows++;
      }
    }
  }

  if (num_rows == 0) {
    // No data, no score.
    return -1;
  }

  // Now, we derive a score, by the simple expedient of using the
  // FFTCompressor to compress the generated transforms, and measuring the
  // length of the resulting bitstream.
  FFTCompressor compressor;
  Datagram dg;
  compressor.write_reals(dg, &i[0], num_rows);
  compressor.write_reals(dg, &j[0], num_rows);
  compressor.write_reals(dg, &k[0], num_rows);
  compressor.write_reals(dg, &a[0], num_rows);
  compressor.write_reals(dg, &b[0], num_rows);
  compressor.write_reals(dg, &c[0], num_rows);
  compressor.write_hprs(dg, &hprs[0], num_rows);
  compressor.write_reals(dg, &x[0], num_rows);
  compressor.write_reals(dg, &y[0], num_rows);
  compressor.write_reals(dg, &z[0], num_rows);


#ifndef HAVE_ZLIB
  return dg.get_length();

#else
  // The FFTCompressor does minimal run-length encoding, but to really get an
  // accurate measure we should zlib-compress the resulting stream.
  std::ostringstream sstr;
  OCompressStream zstr(&sstr, false);
  zstr.write((const char *)dg.get_data(), dg.get_length());
  zstr.flush();
  return sstr.str().length();
#endif
}

/**
 * Calls do_rebuild() on all models, and recursively on all joints at this
 * node and below.  Returns true if all models returned true, false otherwise.
 */
bool EggJointData::
do_rebuild_all(EggCharacterDb &db) {
  bool all_ok = true;

  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != nullptr) {
      EggJointPointer *joint;
      DCAST_INTO_R(joint, back, false);
      if (!joint->do_rebuild(db)) {
        all_ok = false;
      }
    }
  }

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    if (!child->do_rebuild_all(db)) {
      all_ok = false;
    }
  }

  return all_ok;
}

/**
 * Calls optimize() on all models, and recursively on all joints at this node
 * and below.
 */
void EggJointData::
optimize() {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != nullptr) {
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

/**
 * Calls expose() on all models for this joint, but does not recurse
 * downwards.
 */
void EggJointData::
expose(EggGroup::DCSType dcs_type) {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != nullptr) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);
      joint->expose(dcs_type);
    }
  }
}

/**
 * Calls zero_channels() on all models for this joint, but does not recurse
 * downwards.
 */
void EggJointData::
zero_channels(const string &components) {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != nullptr) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);
      joint->zero_channels(components);
    }
  }
}

/**
 * Calls quantize_channels() on all models for this joint, and then recurses
 * downwards to all joints below.
 */
void EggJointData::
quantize_channels(const string &components, double quantum) {
  BackPointers::iterator bpi;
  for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
    EggBackPointer *back = (*bpi);
    if (back != nullptr) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);
      joint->quantize_channels(components, quantum);
    }
  }

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    child->quantize_channels(components, quantum);
  }
}

/**
 * Applies the pose from the indicated frame of the indicated source
 * model_index as the initial pose for this joint, and does this recursively
 * on all joints below.
 */
void EggJointData::
apply_default_pose(int source_model, int frame) {
  if (has_model(source_model)) {
    EggJointPointer *source_joint;
    DCAST_INTO_V(source_joint, _back_pointers[source_model]);
    BackPointers::iterator bpi;
    for (bpi = _back_pointers.begin(); bpi != _back_pointers.end(); ++bpi) {
      EggBackPointer *back = (*bpi);
      if (back != nullptr) {
        EggJointPointer *joint;
        DCAST_INTO_V(joint, back);
        joint->apply_default_pose(source_joint, frame);
      }
    }
  }

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    child->apply_default_pose(source_model, frame);
  }
}

/**
 * Adds the indicated model joint or anim table to the data.
 */
void EggJointData::
add_back_pointer(int model_index, EggObject *egg_object) {
  nassertv(egg_object != nullptr);
  if (egg_object->is_of_type(EggGroup::get_class_type())) {
    // It must be a <Joint>.
    EggJointNodePointer *joint = new EggJointNodePointer(egg_object);
    set_model(model_index, joint);
    if (!_has_rest_frame) {
      _rest_frame = joint->get_frame(0);
      _has_rest_frame = true;

    } else {
      // If this new node doesn't come within an acceptable tolerance of our
      // first reading of this joint's rest frame, set a warning flag.
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

/**
 *
 */
void EggJointData::
write(std::ostream &out, int indent_level) const {
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

/**
 * Clears out the _children vector in preparation for refilling it from the
 * _new_parent information.
 */
void EggJointData::
do_begin_reparent() {
  _got_new_parent_depth = false;
  _children.clear();
}

/**
 * Calculates the number of joints above this joint in its intended position,
 * as specified by a recent call to reparent_to(), and also checks for a cycle
 * in the new parent chain.  Returns true if a cycle is detected, and false
 * otherwise.  If a cycle is not detected, _new_parent_depth can be consulted
 * for the depth in the new hierarchy.
 *
 * This is used by EggCharacterData::do_reparent() to determine the order in
 * which to apply the reparent operations.  It should be called after
 * do_begin_reparent().
 */
bool EggJointData::
calc_new_parent_depth(pset<EggJointData *> &chain) {
  if (_got_new_parent_depth) {
    return false;
  }
  if (_new_parent == nullptr) {
    // Here's the top of the new hierarchy.
    _got_new_parent_depth = true;
    _new_parent_depth = 0;
    return false;
  }
  if (!chain.insert(this).second) {
    // We've already visited this joint; that means there's a cycle.
    return true;
  }
  bool cycle = _new_parent->calc_new_parent_depth(chain);
  _new_parent_depth = _new_parent->_new_parent_depth + 1;
  return cycle;
}

/**
 * Eliminates any cached values before beginning a walk through all the joints
 * for do_compute_reparent(), for a given model/frame.
 */
void EggJointData::
do_begin_compute_reparent() {
  _got_new_net_frame = false;
  _got_new_net_frame_inv = false;
  _computed_reparent = false;
}

/**
 * Prepares the reparent operation by computing a new transform for each frame
 * of each model, designed to keep the net transform the same when the joint
 * is moved to its new parent.  Returns true on success, false on failure.
 */
bool EggJointData::
do_compute_reparent(int model_index, int n, EggCharacterDb &db) {
  if (_computed_reparent) {
    // We've already done this joint.  This is possible because we have to
    // recursively compute joints upwards, so we might visit the same joint
    // more than once.
    return _computed_ok;
  }
  _computed_reparent = true;

  if (_parent == _new_parent) {
    // Trivial (and most common) case: we are not moving the joint.  No
    // recomputation necessary.
    _computed_ok = true;
    return true;
  }

  EggBackPointer *back = get_model(model_index);
  if (back == nullptr) {
    // This joint doesn't have any data to modify.
    _computed_ok = true;
    return true;
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, false);

  LMatrix4d transform;
  if (_parent == nullptr) {
    // We are moving from outside the joint hierarchy to within it.
    transform = _new_parent->get_new_net_frame_inv(model_index, n, db);

  } else if (_new_parent == nullptr) {
    // We are moving from within the hierarchy to outside it.
    transform = _parent->get_net_frame(model_index, n, db);

  } else {
    // We are changing parents within the hierarchy.
    transform =
      _parent->get_net_frame(model_index, n, db) *
      _new_parent->get_new_net_frame_inv(model_index, n, db);
  }

  db.set_matrix(joint, EggCharacterDb::TT_rebuild_frame, n,
                joint->get_frame(n) * transform);
  _computed_ok = true;

  return _computed_ok;
}

/**
 * Calls do_rebuild() on the joint for the indicated model index.  Returns
 * true on success, false on failure (false shouldn't be possible).
 */
bool EggJointData::
do_joint_rebuild(int model_index, EggCharacterDb &db) {
  bool all_ok = true;

  EggJointPointer *parent_joint = nullptr;
  if (_new_parent != nullptr && _new_parent->has_model(model_index)) {
    DCAST_INTO_R(parent_joint, _new_parent->get_model(model_index), false);
  }

  if (has_model(model_index)) {
    EggJointPointer *joint;
    DCAST_INTO_R(joint, get_model(model_index), false);
    if (!joint->do_rebuild(db)) {
      all_ok = false;
    }
  }

  return all_ok;
}

/**
 * Performs the actual reparenting operation by removing all of the old
 * children and replacing them with the set of new children.
 */
void EggJointData::
do_finish_reparent() {
  int num_models = get_num_models();
  for (int model_index = 0; model_index < num_models; model_index++) {
    EggJointPointer *parent_joint = nullptr;
    if (_new_parent != nullptr && _new_parent->has_model(model_index)) {
      DCAST_INTO_V(parent_joint, _new_parent->get_model(model_index));
    }

    if (has_model(model_index)) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, get_model(model_index));
      joint->do_finish_reparent(parent_joint);
    }
  }

  _parent = _new_parent;
  if (_parent != nullptr) {
    _parent->_children.push_back(this);
  }
}

/**
 * Creates a new joint as a child of this joint and returns it.  This is
 * intended to be called only from EggCharacterData::make_new_joint().
 */
EggJointData *EggJointData::
make_new_joint(const string &name) {
  EggJointData *child = new EggJointData(_collection, _char_data);
  child->set_name(name);
  child->_parent = this;
  child->_new_parent = this;
  _children.push_back(child);

  // Also create new back pointers in each of the models.
  int num_models = get_num_models();
  for (int i = 0; i < num_models; i++) {
    if (has_model(i)) {
      EggJointPointer *joint;
      DCAST_INTO_R(joint, get_model(i), nullptr);
      EggJointPointer *new_joint = joint->make_new_joint(name);
      child->set_model(i, new_joint);
    }
  }

  return child;
}

/**
 * The recursive implementation of find_joint, this flavor searches
 * recursively for an exact match of the preferred joint name.
 */
EggJointData *EggJointData::
find_joint_exact(const string &name) {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    if (child->get_name() == name) {
      return child;
    }
    EggJointData *result = child->find_joint_exact(name);
    if (result != nullptr) {
      return result;
    }
  }

  return nullptr;
}

/**
 * The recursive implementation of find_joint, this flavor searches
 * recursively for any acceptable match.
 */
EggJointData *EggJointData::
find_joint_matches(const string &name) {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggJointData *child = (*ci);
    if (child->matches_name(name)) {
      return child;
    }
    EggJointData *result = child->find_joint_matches(name);
    if (result != nullptr) {
      return result;
    }
  }

  return nullptr;
}

/**
 * Returns true if this joint is an ancestor of the indicated joint, in the
 * "new" hierarchy (that is, the one defined by _new_parent, as set by
 * reparent_to() before do_finish_reparent() is called).
 */
bool EggJointData::
is_new_ancestor(EggJointData *child) const {
  if (child == this) {
    return true;
  }

  if (child->_new_parent == nullptr) {
    return false;
  }

  return is_new_ancestor(child->_new_parent);
}

/**
 * Similar to get_net_frame(), but computed for the prospective new parentage
 * of the node, before do_finish_reparent() is called.  This is generally
 * useful only when called within do_compute_reparent().
 */
const LMatrix4d &EggJointData::
get_new_net_frame(int model_index, int n, EggCharacterDb &db) {
  if (!_got_new_net_frame) {
    _new_net_frame = get_new_frame(model_index, n, db);
    if (_new_parent != nullptr) {
      _new_net_frame = _new_net_frame * _new_parent->get_new_net_frame(model_index, n, db);
    }
    _got_new_net_frame = true;
  }
  return _new_net_frame;
}

/**
 * Returns the inverse of get_new_net_frame().
 */
const LMatrix4d &EggJointData::
get_new_net_frame_inv(int model_index, int n, EggCharacterDb &db) {
  if (!_got_new_net_frame_inv) {
    _new_net_frame_inv.invert_from(get_new_frame(model_index, n, db));
    if (_new_parent != nullptr) {
      _new_net_frame_inv = _new_parent->get_new_net_frame_inv(model_index, n, db) * _new_net_frame_inv;
    }
    _got_new_net_frame_inv = true;
  }
  return _new_net_frame_inv;
}

/**
 * Returns the local transform matrix corresponding to this joint position in
 * the nth frame in the indicated model, as it will be when
 * do_finish_reparent() is called.
 */
LMatrix4d EggJointData::
get_new_frame(int model_index, int n, EggCharacterDb &db) {
  do_compute_reparent(model_index, n, db);

  EggBackPointer *back = get_model(model_index);
  if (back == nullptr) {
    return LMatrix4d::ident_mat();
  }

  EggJointPointer *joint;
  DCAST_INTO_R(joint, back, LMatrix4d::ident_mat());

  LMatrix4d mat;
  if (!db.get_matrix(joint, EggCharacterDb::TT_rebuild_frame, n, mat)) {
    // No rebuild frame; return the regular frame.
    return joint->get_frame(n);
  }

  // Return the rebuild frame, as computed.
  return mat;
}
