// Filename: modelFlattenRequest.I
// Created by:  drose (30Mar07)
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
