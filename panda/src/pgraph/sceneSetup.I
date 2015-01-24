// Filename: sceneSetup.I
// Created by:  drose (27Mar02)
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
//     Function: SceneSetup::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SceneSetup::
SceneSetup() {
  _display_region = NULL;
  _viewport_width = 0;
  _viewport_height = 0;
  _inverted = false;
  _initial_state = RenderState::make_empty();
  _camera_transform = TransformState::make_identity();
  _world_transform = TransformState::make_identity();
  _cs_transform = TransformState::make_identity();
  _cs_world_transform = TransformState::make_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_display_region
//       Access: Published
//  Description: Specifies the display region for the scene.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_display_region(DisplayRegion *display_region) {
  _display_region = display_region;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_display_region
//       Access: Published
//  Description: Returns the display region for the scene.
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion *SceneSetup::
get_display_region() const {
  return _display_region;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_viewport_size
//       Access: Published
//  Description: Specifies the size of the viewport (display region),
//               in pixels.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_viewport_size(int width, int height) {
  _viewport_width = width;
  _viewport_height = height;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_viewport_width
//       Access: Published
//  Description: Returns the width of the viewport (display region) in
//               pixels.
////////////////////////////////////////////////////////////////////
INLINE int SceneSetup::
get_viewport_width() const {
  return _viewport_width;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_viewport_height
//       Access: Published
//  Description: Returns the height of the viewport (display region) in
//               pixels.
////////////////////////////////////////////////////////////////////
INLINE int SceneSetup::
get_viewport_height() const {
  return _viewport_height;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_scene_root
//       Access: Published
//  Description: Specifies the root node of the scene.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_scene_root(const NodePath &scene_root) {
  _scene_root = scene_root;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_scene_root
//       Access: Published
//  Description: Returns the root node of the scene.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &SceneSetup::
get_scene_root() const {
  return _scene_root;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_camera_path
//       Access: Published
//  Description: Specifies the NodePath to the camera.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_camera_path(const NodePath &camera_path) {
  _camera_path = camera_path;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_camera_path
//       Access: Published
//  Description: Returns the NodePath to the camera.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &SceneSetup::
get_camera_path() const {
  return _camera_path;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_camera_node
//       Access: Published
//  Description: Specifies the camera used to render the scene.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_camera_node(Camera *camera_node) {
  _camera_node = camera_node;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_camera_node
//       Access: Published
//  Description: Returns the camera used to render the scene.
////////////////////////////////////////////////////////////////////
INLINE Camera *SceneSetup::
get_camera_node() const {
  return _camera_node;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_lens
//       Access: Published
//  Description: Indicates the particular Lens used for rendering.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_lens(const Lens *lens) {
  _lens = lens;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_lens
//       Access: Published
//  Description: Returns the particular Lens used for rendering.
////////////////////////////////////////////////////////////////////
INLINE const Lens *SceneSetup::
get_lens() const {
  return _lens;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_inverted
//       Access: Published
//  Description: Changes the current setting of the inverted flag.
//               When this is true, the scene is rendered into the
//               window upside-down and backwards, that is, inverted
//               as if viewed through a mirror placed on the floor.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_inverted(bool inverted) {
  _inverted = inverted;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_inverted
//       Access: Published
//  Description: Returns the current setting of the inverted flag.
//               When this is true, the scene is rendered into the
//               window upside-down, flipped like a mirror along the X
//               axis.
////////////////////////////////////////////////////////////////////
INLINE bool SceneSetup::
get_inverted() const {
  return _inverted;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_cull_center
//       Access: Published
//  Description: Returns the point from which the culling operations
//               will be performed.  This is normally the camera, but
//               if camera->set_cull_center() has been specified, it
//               will be that special node instead.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &SceneSetup::
get_cull_center() const {
  if (_camera_node->get_cull_center().is_empty()) {
    return _camera_path;
  } else {
    return _camera_node->get_cull_center();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_cull_bounds
//       Access: Published
//  Description: Returns the bounding volume that should be used to
//               perform view-frustum culling (in the space of
//               get_cull_center()).  This is normally the current
//               lens' bounding volume, but it may be overridden with
//               Camera::set_cull_bounds().
////////////////////////////////////////////////////////////////////
INLINE PT(BoundingVolume) SceneSetup::
get_cull_bounds() const {
  PT(BoundingVolume) bounds = _camera_node->get_cull_bounds();
  if (bounds != (BoundingVolume *)NULL) {
    return bounds;
  }

  return _lens->make_bounds();
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_initial_state
//       Access: Published
//  Description: Sets the initial state which is applied to all nodes
//               in the scene, as if it were set at the top of the
//               scene graph.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_initial_state(const RenderState *state) {
  _initial_state = state;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_initial_state
//       Access: Published
//  Description: Returns the initial state as set by a previous call
//               to set_initial_state().
////////////////////////////////////////////////////////////////////
INLINE const RenderState *SceneSetup::
get_initial_state() const {
  return _initial_state;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_camera_transform
//       Access: Published
//  Description: Specifies the position of the camera relative to the
//               starting node.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_camera_transform(const TransformState *camera_transform) {
  _camera_transform = camera_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_camera_transform
//       Access: Published
//  Description: Returns the position of the camera relative to the
//               starting node.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *SceneSetup::
get_camera_transform() const {
  return _camera_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_world_transform
//       Access: Published
//  Description: Specifies the position of the starting node relative
//               to the camera.  This is the inverse of the camera
//               transform.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_world_transform(const TransformState *world_transform) {
  _world_transform = world_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_world_transform
//       Access: Published
//  Description: Returns the position of the starting node relative
//               to the camera.  This is the inverse of the camera
//               transform.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *SceneSetup::
get_world_transform() const {
  return _world_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_cs_transform
//       Access: Published
//  Description: Specifies the transform from the camera's coordinate
//               system to the GSG's internal coordinate system.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_cs_transform(const TransformState *cs_transform) {
  _cs_transform = cs_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_cs_transform
//       Access: Published
//  Description: Returns the transform from the camera's coordinate
//               system to the GSG's internal coordinate system.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *SceneSetup::
get_cs_transform() const {
  return _cs_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_cs_world_transform
//       Access: Published
//  Description: Specifies the position from the starting node
//               relative to the camera, in the GSG's internal
//               coordinate system.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_cs_world_transform(const TransformState *cs_world_transform) {
  _cs_world_transform = cs_world_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_cs_world_transform
//       Access: Published
//  Description: Returns the position from the starting node
//               relative to the camera, in the GSG's internal
//               coordinate system.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *SceneSetup::
get_cs_world_transform() const {
  return _cs_world_transform;
}
