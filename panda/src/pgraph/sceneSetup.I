// Filename: sceneSetup.I
// Created by:  drose (27Mar02)
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
//     Function: SceneSetup::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SceneSetup::
SceneSetup() {
  _inverted = false;
  _initial_state = RenderState::make_empty();
  _camera_transform = TransformState::make_identity();
  _world_transform = TransformState::make_identity();
  _cs_transform = TransformState::make_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_scene_root
//       Access: Public
//  Description: Specifies the root node of the scene.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_scene_root(const NodePath &scene_root) {
  _scene_root = scene_root;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_scene_root
//       Access: Public
//  Description: Returns the root node of the scene.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &SceneSetup::
get_scene_root() const {
  return _scene_root;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_camera_path
//       Access: Public
//  Description: Specifies the NodePath to the camera.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_camera_path(const NodePath &camera_path) {
  _camera_path = camera_path;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_camera_path
//       Access: Public
//  Description: Returns the NodePath to the camera.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &SceneSetup::
get_camera_path() const {
  return _camera_path;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_camera_node
//       Access: Public
//  Description: Specifies the camera used to render the scene.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_camera_node(Camera *camera_node) {
  _camera_node = camera_node;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_camera_node
//       Access: Public
//  Description: Returns the camera used to render the scene.
////////////////////////////////////////////////////////////////////
INLINE Camera *SceneSetup::
get_camera_node() const {
  return _camera_node;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_lens
//       Access: Public
//  Description: Indicates the particular Lens used for rendering.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_lens(const Lens *lens) {
  _lens = lens;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_lens
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: Specifies the position of the camera relative to the
//               starting node, without any compensating
//               coordinate-system transforms that might have been
//               introduced for the purposes of rendering.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_camera_transform(const TransformState *camera_transform) {
  _camera_transform = camera_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_camera_transform
//       Access: Public
//  Description: Returns the position of the camera relative to the
//               starting node, without any compensating
//               coordinate-system transforms that might have been
//               introduced for the purposes of rendering.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *SceneSetup::
get_camera_transform() const {
  return _camera_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_world_transform
//       Access: Public
//  Description: Specifies the position of the starting node relative
//               to the camera.  This is the inverse of the camera
//               transform.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_world_transform(const TransformState *world_transform) {
  _world_transform = world_transform;
  _render_transform = _cs_transform->compose(_world_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_world_transform
//       Access: Public
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
//       Access: Public
//  Description: Specifies the pretransformation to apply to the world
//               transform to produce the appropriate transformation
//               for rendering.  This is usually the appropriate
//               coordinate-system conversion for the current GSG.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_cs_transform(const TransformState *cs_transform) {
  _cs_transform = cs_transform;
  _render_transform = _cs_transform->compose(_world_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_cs_transform
//       Access: Public
//  Description: Returns the pretransformation to apply to the world
//               transform to produce the appropriate transformation
//               for rendering.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *SceneSetup::
get_cs_transform() const {
  return _cs_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_render_transform
//       Access: Public
//  Description: Returns the position of the starting node relative
//               to the camera, pretransformed as appropriate for
//               rendering.  This is the same as the world transform,
//               with a possible coordinate-system conversion applied.
//
//               Note that this value is always the position of the
//               starting node, not the current node, even if it is
//               sampled during a traversal.  To get the render
//               transform of the current node check in the current
//               CullTraverserData.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *SceneSetup::
get_render_transform() const {
  return _render_transform;
}
