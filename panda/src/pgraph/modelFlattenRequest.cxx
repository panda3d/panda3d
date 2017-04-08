/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelFlattenRequest.cxx
 * @author drose
 * @date 2007-03-30
 */

#include "modelFlattenRequest.h"
#include "nodePath.h"

TypeHandle ModelFlattenRequest::_type_handle;

/**
 * Performs the task: that is, copies and flattens the model.
 */
AsyncTask::DoneStatus ModelFlattenRequest::
do_task() {
  // We make another instance of the original node, so we can safely flatten
  // that without affecting the original copy.
  NodePath np("flatten_root");

  // Except if we try to attach a node without parents, this will cause the
  // original NodePath to be affected, so we have to make a (shallow) copy.
  if (_orig->get_num_parents() == 0) {
    PT(PandaNode) copy = _orig->make_copy();
    copy->copy_children(_orig);
    np.attach_new_node(copy);
  } else {
    np.attach_new_node(_orig);
  }
  np.flatten_strong();
  _model = np.get_child(0).node();
  _is_ready = true;

  // Don't continue the task; we're done.
  return DS_done;
}
