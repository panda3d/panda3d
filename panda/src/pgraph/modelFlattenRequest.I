/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelFlattenRequest.I
 * @author drose
 * @date 2007-03-30
 */

/**
 * Create a new ModelFlattenRequest, and add it to the loader via
 * load_async(), to begin an asynchronous load.
 */
INLINE ModelFlattenRequest::
ModelFlattenRequest(PandaNode *orig) :
  AsyncTask(orig->get_name()),
  _orig(orig),
  _is_ready(false)
{
}

/**
 * Returns the original, unflattened node.
 */
INLINE PandaNode *ModelFlattenRequest::
get_orig() const {
  return _orig;
}

/**
 * Returns true if this request has completed, false if it is still pending.
 * When this returns true, you may retrieve the model loaded by calling
 * result().
 */
INLINE bool ModelFlattenRequest::
is_ready() const {
  return _is_ready;
}

/**
 * Returns the flattened copy of the model.  It is an error to call this
 * unless is_ready() returns true.
 */
INLINE PandaNode *ModelFlattenRequest::
get_model() const {
  nassertr(_is_ready, nullptr);
  return _model;
}

/**
 * Returns the flattened copy of the model wrapped in a NodePath.  It is an
 * error to call this unless is_ready() returns true.
 */
INLINE NodePath ModelFlattenRequest::
result() const {
  nassertr(_is_ready, NodePath::fail());
  if (_model != nullptr) {
    return NodePath(_model);
  } else {
    return NodePath::fail();
  }
}
