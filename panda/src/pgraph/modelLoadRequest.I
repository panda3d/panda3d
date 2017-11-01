/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelLoadRequest.I
 * @author drose
 * @date 2006-08-29
 */

/**
 * Returns the filename associated with this asynchronous ModelLoadRequest.
 */
INLINE const Filename &ModelLoadRequest::
get_filename() const {
  return _filename;
}

/**
 * Returns the LoaderOptions associated with this asynchronous
 * ModelLoadRequest.
 */
INLINE const LoaderOptions &ModelLoadRequest::
get_options() const {
  return _options;
}

/**
 * Returns the Loader object associated with this asynchronous
 * ModelLoadRequest.
 */
INLINE Loader *ModelLoadRequest::
get_loader() const {
  return _loader;
}

/**
 * Returns the model that was loaded asynchronously as a NodePath, if any, or
 * the empty NodePath if there was an error.
 */
INLINE NodePath ModelLoadRequest::
result() const {
  nassertr_always(_is_ready, NodePath::fail());
  return NodePath(_model);
}

/**
 * Returns true if this request has completed, false if it is still pending.
 * When this returns true, you may retrieve the model loaded by calling
 * get_model().
 */
INLINE bool ModelLoadRequest::
is_ready() const {
  return _is_ready;
}

/**
 * Returns the model that was loaded asynchronously, if any, or NULL if there
 * was an error.  It is an error to call this unless is_ready() returns true.
 */
INLINE PandaNode *ModelLoadRequest::
get_model() const {
  nassertr(_is_ready, NULL);
  return _model;
}
