// Filename: camera.I
// Created by:  drose (26Feb02)
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
//     Function: Camera::set_active
//       Access: Published
//  Description: Sets the active flag on the camera.  When the camera
//               is not active, nothing will be rendered.
////////////////////////////////////////////////////////////////////
INLINE void Camera::
set_active(bool active) {
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::is_active
//       Access: Published
//  Description: Returns the current setting of the active flag on the
//               camera.
////////////////////////////////////////////////////////////////////
INLINE bool Camera::
is_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::set_scene
//       Access: Published
//  Description: Sets the scene that will be rendered by the camera.
//               This is normally the root node of a scene graph,
//               typically a node called 'render', although it could
//               represent the root of any subgraph.
////////////////////////////////////////////////////////////////////
INLINE void Camera::
set_scene(const NodePath &scene) {
  _scene = scene;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_scene
//       Access: Published
//  Description: Returns the scene that will be rendered by the
//               camera.  See set_scene().
////////////////////////////////////////////////////////////////////
INLINE const NodePath &Camera::
get_scene() const {
  return _scene;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_num_display_regions
//       Access: Published
//  Description: Returns the number of display regions associated with
//               the camera.
////////////////////////////////////////////////////////////////////
INLINE int Camera::
get_num_display_regions() const {
  return _display_regions.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_display_region
//       Access: Published
//  Description: Returns the nth display region associated with the
//               camera.
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion *Camera::
get_display_region(int n) const {
  nassertr(n >= 0 && n < (int)_display_regions.size(), (DisplayRegion *)NULL);
  return _display_regions[n];
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::set_camera_mask
//       Access: Published
//  Description: Changes the set of bits that represent the subset of
//               the scene graph the camera will render.
//
//               During the cull traversal, a node is not visited if
//               none of its draw mask bits intersect with the
//               camera's camera mask bits.  These masks can be used
//               to selectively hide and show different parts of the
//               scene graph from different cameras that are otherwise
//               viewing the same scene.
////////////////////////////////////////////////////////////////////
INLINE void Camera::
set_camera_mask(DrawMask mask) {
  _camera_mask = mask;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_camera_mask
//       Access: Published
//  Description: Returns the set of bits that represent the subset of
//               the scene graph the camera will render.  See
//               set_camera_mask().
////////////////////////////////////////////////////////////////////
INLINE DrawMask Camera::
get_camera_mask() const {
  return _camera_mask;
}
