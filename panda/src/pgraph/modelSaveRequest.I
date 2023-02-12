/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelSaveRequest.I
 * @author drose
 * @date 2012-12-19
 */

/**
 * Returns the filename associated with this asynchronous ModelSaveRequest.
 */
INLINE const Filename &ModelSaveRequest::
get_filename() const {
  return _filename;
}

/**
 * Returns the LoaderOptions associated with this asynchronous
 * ModelSaveRequest.
 */
INLINE const LoaderOptions &ModelSaveRequest::
get_options() const {
  return _options;
}

/**
 * Returns the node that was passed to the constructor.
 */
INLINE PandaNode *ModelSaveRequest::
get_node() const {
  return _node;
}

/**
 * Returns the Loader object associated with this asynchronous
 * ModelSaveRequest.
 */
INLINE Loader *ModelSaveRequest::
get_loader() const {
  return _loader;
}

/**
 * Returns true if this request has completed, false if it is still pending.
 * When this returns true, you may retrieve the success flag with
 * get_success().
 * Equivalent to `req.done() and not req.cancelled()`.
 * @see done()
 */
INLINE bool ModelSaveRequest::
is_ready() const {
  return (FutureState)_future_state.load(std::memory_order_relaxed) == FS_finished;
}

/**
 * Returns the true if the model was saved successfully, false otherwise.  It
 * is an error to call this unless done() returns true.
 */
INLINE bool ModelSaveRequest::
get_success() const {
  nassertr_always(done(), false);
  return _success;
}
