// Filename: sceneSetup.I
// Created by:  drose (27Mar02)
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
//     Function: SceneSetup::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SceneSetup::
SceneSetup() {
  _camera_transform = TransformState::make_identity();
  _render_transform = TransformState::make_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_scene_root
//       Access: Public
//  Description: Specifies the root node of the scene.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_scene_root(const qpNodePath &scene_root) {
  _scene_root = scene_root;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_scene_root
//       Access: Public
//  Description: Returns the root node of the scene.
////////////////////////////////////////////////////////////////////
INLINE const qpNodePath &SceneSetup::
get_scene_root() const {
  return _scene_root;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::set_camera
//       Access: Public
//  Description: Specifies the camera used to render the scene.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_camera(const qpCamera *camera) {
  _camera = camera;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_camera
//       Access: Public
//  Description: Returns the camera used to render the scene.
////////////////////////////////////////////////////////////////////
INLINE const qpCamera *SceneSetup::
get_camera() const {
  return _camera;
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
//     Function: SceneSetup::set_render_transform
//       Access: Public
//  Description: Specifies the position of the starting node relative
//               to the camera, pretransformed as appropriate for
//               rendering.
////////////////////////////////////////////////////////////////////
INLINE void SceneSetup::
set_render_transform(const TransformState *render_transform) {
  _render_transform = render_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneSetup::get_render_transform
//       Access: Public
//  Description: Returns the position of the starting node relative
//               to the camera, pretransformed as appropriate for
//               rendering.
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
