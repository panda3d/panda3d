// Filename: modelLoadRequest.I
// Created by:  drose (29Aug06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ModelLoadRequest::Constructor
//       Access: Published
//  Description: Create a new ModelLoadRequest, and add it to the loader
//               via load_async(), to begin an asynchronous load.
////////////////////////////////////////////////////////////////////
INLINE ModelLoadRequest::
ModelLoadRequest(const Filename &filename, const LoaderOptions &options) :
  _filename(filename),
  _options(options),
  _is_ready(false)
{
}

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
