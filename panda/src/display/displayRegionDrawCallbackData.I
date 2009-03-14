// Filename: displayRegionDrawCallbackData.I
// Created by:  drose (13Mar09)
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
//     Function: DisplayRegionDrawCallbackData::get_cull_result
//       Access: Published
//  Description: Returns a pointer to the CullResult, the list of
//               CullableObjects that should be drawn in this
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE CullResult *DisplayRegionDrawCallbackData::
get_cull_result() const {
  return _cull_result;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionDrawCallbackData::get_scene_setup
//       Access: Published
//  Description: Returns a pointer to the SceneSetup object, which
//               contains information about the camera and such.
////////////////////////////////////////////////////////////////////
INLINE SceneSetup *DisplayRegionDrawCallbackData::
get_scene_setup() const {
  return _scene_setup;
}
