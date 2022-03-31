/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMatrixTablePointer.cxx
 * @author drose
 * @date 2001-02-26
 */

#include "eggMatrixTablePointer.h"

#include "dcast.h"
#include "eggCharacterDb.h"
#include "eggSAnimData.h"
#include "eggXfmAnimData.h"
#include "eggXfmSAnim.h"

using std::string;

TypeHandle EggMatrixTablePointer::_type_handle;

/**
 *
 */
EggMatrixTablePointer::
EggMatrixTablePointer(EggObject *object) {
  _table = DCAST(EggTable, object);

  if (_table != nullptr) {
    // Now search for the child named "xform".  This contains the actual table
    // data.
    EggGroupNode::iterator ci;
    bool found = false;
    for (ci = _table->begin(); ci != _table->end() && !found; ++ci) {
      EggNode *child = (*ci);
      if (child->get_name() == "xform") {
        if (child->is_of_type(EggXfmSAnim::get_class_type())) {
          _xform = DCAST(EggXfmSAnim, child);
          _xform->normalize();
          found = true;

        } else if (child->is_of_type(EggXfmAnimData::get_class_type())) {
          // Quietly replace old-style XfmAnim tables with new-style XfmSAnim
          // tables.
          PT(EggXfmAnimData) anim = DCAST(EggXfmAnimData, child);
          _xform = new EggXfmSAnim(*anim);
          _table->replace(ci, _xform.p());
          found = true;
        }
      }
    }
  }
}

/**
 * Returns the stated frame rate of this particular joint, or 0.0 if it
 * doesn't state.
 */
double EggMatrixTablePointer::
get_frame_rate() const {
  if (_xform == nullptr || !_xform->has_fps()) {
    return 0.0;
  } else {
    return _xform->get_fps();
  }
}

/**
 * Returns the number of frames of animation for this particular joint.
 */
int EggMatrixTablePointer::
get_num_frames() const {
  if (_xform == nullptr) {
    return 0;
  } else {
    return _xform->get_num_rows();
  }
}

/**
 * Extends the table to the indicated number of frames.
 */
void EggMatrixTablePointer::
extend_to(int num_frames) {
  nassertv(_xform != nullptr);
  _xform->normalize();
  int num_rows = _xform->get_num_rows();
  LMatrix4d last_mat;
  if (num_rows == 0) {
    last_mat = LMatrix4d::ident_mat();
  } else {
    _xform->get_value(num_rows - 1, last_mat);
  }

  while (num_rows < num_frames) {
    _xform->add_data(last_mat);
    num_rows++;
  }
}

/**
 * Returns the transform matrix corresponding to this joint position in the
 * nth frame.
 */
LMatrix4d EggMatrixTablePointer::
get_frame(int n) const {
  if (get_num_frames() == 1) {
    // If we have exactly one frame, then we have as many frames as we want;
    // just repeat the first frame.
    n = 0;

  } else if (get_num_frames() == 0) {
    // If we have no frames, we really have the identity matrix.
    return LMatrix4d::ident_mat();
  }

  nassertr(n >= 0 && n < get_num_frames(), LMatrix4d::ident_mat());
  LMatrix4d mat;
  _xform->get_value(n, mat);
  return mat;
}

/**
 * Sets the transform matrix corresponding to this joint position in the nth
 * frame.
 */
void EggMatrixTablePointer::
set_frame(int n, const LMatrix4d &mat) {
  nassertv(n >= 0 && n < get_num_frames());
  _xform->set_value(n, mat);
}

/**
 * Appends a new frame onto the end of the data, if possible; returns true if
 * not possible, or false otherwise (e.g.  for a static joint).
 */
bool EggMatrixTablePointer::
add_frame(const LMatrix4d &mat) {
  if (_xform == nullptr) {
    return false;
  }

  return _xform->add_data(mat);
}

/**
 * Performs the actual reparenting operation by removing the node from its old
 * parent and associating it with its new parent, if any.
 */
void EggMatrixTablePointer::
do_finish_reparent(EggJointPointer *new_parent) {
  if (new_parent == nullptr) {
    // No new parent; unparent the joint.
    EggGroupNode *egg_parent = _table->get_parent();
    if (egg_parent != nullptr) {
      egg_parent->remove_child(_table.p());
    }

  } else {
    // Reparent the joint to its new parent (implicitly unparenting it from
    // its previous parent).
    EggMatrixTablePointer *new_node = DCAST(EggMatrixTablePointer, new_parent);
    if (new_node->_table != _table->get_parent()) {
      new_node->_table->add_child(_table.p());
    }
  }
}

/**
 * Rebuilds the entire table all at once, based on the frames added by
 * repeated calls to add_rebuild_frame() since the last call to
 * begin_rebuild().
 *
 * Until do_rebuild() is called, the animation table is not changed.
 *
 * The return value is true if all frames are acceptable, or false if there is
 * some problem.
 */
bool EggMatrixTablePointer::
do_rebuild(EggCharacterDb &db) {
  LMatrix4d mat;
  if (!db.get_matrix(this, EggCharacterDb::TT_rebuild_frame, 0, mat)) {
    // No rebuild frame; this is OK.
    return true;
  }

  if (_xform == nullptr) {
    return false;
  }

  bool all_ok = true;

  _xform->clear_data();
  if (!_xform->add_data(mat)) {
    all_ok = false;
  }

  // Assume all frames will be contiguous.
  int n = 1;
  while (db.get_matrix(this, EggCharacterDb::TT_rebuild_frame, n, mat)) {
    if (!_xform->add_data(mat)) {
      all_ok = false;
    }
    ++n;
  }

  return all_ok;
}

/**
 * Resets the table before writing to disk so that redundant rows (e.g.  i { 1
 * 1 1 1 1 1 1 1 }) are collapsed out.
 */
void EggMatrixTablePointer::
optimize() {
  if (_xform != nullptr) {
    _xform->optimize();
  }
}

/**
 * Zeroes out the named components of the transform in the animation frames.
 */
void EggMatrixTablePointer::
zero_channels(const string &components) {
  if (_xform == nullptr) {
    return;
  }

  // This is particularly easy: we only have to remove children from the
  // _xform object whose name is listed in the components.
  string::const_iterator si;
  for (si = components.begin(); si != components.end(); ++si) {
    string table_name(1, *si);
    EggNode *child = _xform->find_child(table_name);
    if (child != nullptr) {
      _xform->remove_child(child);
    }
  }
}

/**
 * Rounds the named components of the transform to the nearest multiple of
 * quantum.
 */
void EggMatrixTablePointer::
quantize_channels(const string &components, double quantum) {
  if (_xform == nullptr) {
    return;
  }

  // This is similar to the above: we quantize children of the _xform object
  // whose name is listed in the components.
  string::const_iterator si;
  for (si = components.begin(); si != components.end(); ++si) {
    string table_name(1, *si);
    EggNode *child = _xform->find_child(table_name);
    if (child != nullptr &&
        child->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *anim = DCAST(EggSAnimData, child);
      anim->quantize(quantum);
    }
  }
}

/**
 * Creates a new child of the current joint in the egg data, and returns a
 * pointer to it.
 */
EggJointPointer *EggMatrixTablePointer::
make_new_joint(const string &name) {
  EggTable *new_table = new EggTable(name);
  _table->add_child(new_table);
  CoordinateSystem cs = CS_default;
  if (_xform != nullptr) {
    cs = _xform->get_coordinate_system();
  }
  EggXfmSAnim *new_xform = new EggXfmSAnim("xform", cs);
  new_table->add_child(new_xform);
  new_xform->add_data(LMatrix4d::ident_mat());

  return new EggMatrixTablePointer(new_table);
}

/**
 * Applies the indicated name change to the egg file.
 */
void EggMatrixTablePointer::
set_name(const string &name) {
  _table->set_name(name);
}
