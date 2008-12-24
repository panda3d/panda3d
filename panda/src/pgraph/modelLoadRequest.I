// Filename: modelLoadRequest.I
// Created by:  drose (29Aug06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ModelLoadRequest::get_filename
//       Access: Published
//  Description: Returns the filename associated with this
//               asynchronous ModelLoadRequest.
////////////////////////////////////////////////////////////////////
INLINE const Filename &ModelLoadRequest::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelLoadRequest::get_options
//       Access: Published
//  Description: Returns the LoaderOptions associated with this
//               asynchronous ModelLoadRequest.
////////////////////////////////////////////////////////////////////
INLINE const LoaderOptions &ModelLoadRequest::
get_options() const {
  return _options;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelLoadRequest::get_loader
//       Access: Published
//  Description: Returns the Loader object associated with this
//               asynchronous ModelLoadRequest.
////////////////////////////////////////////////////////////////////
INLINE Loader *ModelLoadRequest::
get_loader() const {
  return _loader;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelLoadRequest::is_ready
//       Access: Published
//  Description: Returns true if this request has completed, false if
//               it is still pending.  When this returns true, you may
//               retrieve the model loaded by calling get_model().
////////////////////////////////////////////////////////////////////
INLINE bool ModelLoadRequest::
is_ready() const {
  return _is_ready;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelLoadRequest::get_model
//       Access: Published
//  Description: Returns the model that was loaded asynchronously, if
//               any, or NULL if there was an error.  It is an error
//               to call this unless is_ready() returns true.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *ModelLoadRequest::
get_model() const {
  nassertr(_is_ready, NULL);
  return _model;
}
