// Filename: modelFlattenRequest.I
// Created by:  drose (30Mar07)
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
//     Function: ModelFlattenRequest::Constructor
//       Access: Published
//  Description: Create a new ModelFlattenRequest, and add it to the loader
//               via load_async(), to begin an asynchronous load.
////////////////////////////////////////////////////////////////////
INLINE ModelFlattenRequest::
ModelFlattenRequest(PandaNode *orig) :
  AsyncTask(orig->get_name()),
  _orig(orig),
  _is_ready(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ModelFlattenRequest::get_orig
//       Access: Published
//  Description: Returns the original, unflattened node.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *ModelFlattenRequest::
get_orig() const {
  return _orig;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelFlattenRequest::is_ready
//       Access: Published
//  Description: Returns true if this request has completed, false if
//               it is still pending.  When this returns true, you may
//               retrieve the model loaded by calling get_result().
////////////////////////////////////////////////////////////////////
INLINE bool ModelFlattenRequest::
is_ready() const {
  return _is_ready;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelFlattenRequest::get_model
//       Access: Published
//  Description: Returns the flattened copy of the model.  It is an
//               error to call this unless is_ready() returns true.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *ModelFlattenRequest::
get_model() const {
  nassertr(_is_ready, NULL);
  return _model;
}
