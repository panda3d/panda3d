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

////////////////////////////////////////////////////////////////////
//     Function: Camera::set_cull_center
//       Access: Published
//  Description: Specifies the point from which the culling operations
//               are performed.  Normally, this is the same as the
//               camera, and that is the default if this is not
//               specified; but it may sometimes be useful to perform
//               the culling from some other viewpoint, particularly
//               when you are debugging the culling itself.
////////////////////////////////////////////////////////////////////
INLINE void Camera::
set_cull_center(const NodePath &cull_center) {
  _cull_center = cull_center;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_cull_center
//       Access: Published
//  Description: Returns the point from which the culling operations
//               will be performed, if it was set by
//               set_cull_center(), or the empty NodePath otherwise.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &Camera::
get_cull_center() const {
  return _cull_center;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::set_initial_state
//       Access: Published
//  Description: Sets the initial state which is applied to all nodes
//               in the scene, as if it were set at the top of the
//               scene graph.
////////////////////////////////////////////////////////////////////
INLINE void Camera::
set_initial_state(const RenderState *state) {
  _initial_state = state;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_initial_state
//       Access: Published
//  Description: Returns the initial state as set by a previous call
//               to set_initial_state().
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderState) Camera::
get_initial_state() const {
  return _initial_state;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::set_tag_state_key
//       Access: Published
//  Description: Sets the tag key which, when encountered as a tag on
//               nodes in the scene graph, causes this Camera to apply
//               an arbitrary state transition based on the value of
//               the tag (as specified to set_tag_state()).
////////////////////////////////////////////////////////////////////
INLINE void Camera::
set_tag_state_key(const string &tag_state_key) {
  _tag_state_key = tag_state_key;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_tag_state_key
//       Access: Published
//  Description: Returns the tag key as set by a previous call to
//               set_tag_state_key().
////////////////////////////////////////////////////////////////////
INLINE const string &Camera::
get_tag_state_key() const {
  return _tag_state_key;
}
