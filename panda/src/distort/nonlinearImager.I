// Filename: nonlinearImager.I
// Created by:  drose (12Dec01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::set_camera
//       Access: Published
//  Description: Specifies the virtual camera that will be used to
//               view the various ProjectionScreens.  It should be in
//               the same scene graph with the ProjectionScreens, to
//               establish a relative coordinate system with them.
////////////////////////////////////////////////////////////////////
INLINE void NonlinearImager::
set_camera(LensNode *camera) {
  _camera = camera;
  _stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_camera
//       Access: Published
//  Description: Returns the virtual camera that will be used to
//               view the various ProjectionScreens.
////////////////////////////////////////////////////////////////////
INLINE LensNode *NonlinearImager::
get_camera() const {
  return _camera;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_internal_scene
//       Access: Published
//  Description: Returns a pointer to the root node of the internal
//               scene graph, which is used to render all of the
//               screen meshes.
////////////////////////////////////////////////////////////////////
INLINE NodePath NonlinearImager::
get_internal_scene() const {
  return _internal_scene;
}
