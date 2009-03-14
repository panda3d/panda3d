// Filename: displayRegionCullCallbackData.I
// Created by:  drose (14Mar09)
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
//     Function: DisplayRegionCullCallbackData::get_cull_handler
//       Access: Published
//  Description: Returns a pointer to the CullHandler, which accepts
//               each object to be added to the list for drawing.
////////////////////////////////////////////////////////////////////
INLINE CullHandler *DisplayRegionCullCallbackData::
get_cull_handler() const {
  return _cull_handler;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionCullCallbackData::get_scene_setup
//       Access: Published
//  Description: Returns a pointer to the SceneSetup object, which
//               contains information about the camera and such.
////////////////////////////////////////////////////////////////////
INLINE SceneSetup *DisplayRegionCullCallbackData::
get_scene_setup() const {
  return _scene_setup;
}
