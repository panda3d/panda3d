// Filename: modelSaveRequest.I
// Created by:  drose (19Dec12)
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
//     Function: ModelSaveRequest::get_filename
//       Access: Published
//  Description: Returns the filename associated with this
//               asynchronous ModelSaveRequest.
////////////////////////////////////////////////////////////////////
INLINE const Filename &ModelSaveRequest::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelSaveRequest::get_options
//       Access: Published
//  Description: Returns the LoaderOptions associated with this
//               asynchronous ModelSaveRequest.
////////////////////////////////////////////////////////////////////
INLINE const LoaderOptions &ModelSaveRequest::
get_options() const {
  return _options;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelSaveRequest::get_node
//       Access: Published
//  Description: Returns the node that was passed to the constructor.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *ModelSaveRequest::
get_node() const {
  return _node;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelSaveRequest::get_loader
//       Access: Published
//  Description: Returns the Loader object associated with this
//               asynchronous ModelSaveRequest.
////////////////////////////////////////////////////////////////////
INLINE Loader *ModelSaveRequest::
get_loader() const {
  return _loader;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelSaveRequest::is_ready
//       Access: Published
//  Description: Returns true if this request has completed, false if
//               it is still pending.  When this returns true, you may
//               retrieve the success flag with get_success().
////////////////////////////////////////////////////////////////////
INLINE bool ModelSaveRequest::
is_ready() const {
  return _is_ready;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelSaveRequest::get_success
//       Access: Published
//  Description: Returns the true if the model was saved successfully,
//               false otherwise.  It is an error to call this unless
//               is_ready() returns true.
////////////////////////////////////////////////////////////////////
INLINE bool ModelSaveRequest::
get_success() const {
  nassertr(_is_ready, false);
  return _success;
}
