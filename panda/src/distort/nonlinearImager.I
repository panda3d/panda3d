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
//     Function: NonlinearImager::get_viewer
//       Access: Published
//  Description: Returns the NodePath to the LensNode that is to serve
//               as the viewer for this screen, or empty if no
//               viewer is associated.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &NonlinearImager::
get_viewer() const {
  return _viewer;
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
