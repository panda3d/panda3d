/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file camera.I
 * @author drose
 * @date 2002-02-26
 */

/**
 * Sets the active flag on the camera.  When the camera is not active, nothing
 * will be rendered.
 */
INLINE void Camera::
set_active(bool active) {
  _active = active;
}

/**
 * Returns the current setting of the active flag on the camera.
 */
INLINE bool Camera::
is_active() const {
  return _active;
}

/**
 * Sets the scene that will be rendered by the camera.  This is normally the
 * root node of a scene graph, typically a node called 'render', although it
 * could represent the root of any subgraph.
 *
 * Note that the use of this method is now deprecated.  In the absence of an
 * explicit scene set on the camera, the camera will render whatever scene it
 * is parented into.  This is the preferred way to specify the scene, since it
 * is the more intuitive mechanism.
 */
INLINE void Camera::
set_scene(const NodePath &scene) {
  _scene = scene;
}

/**
 * Returns the scene that will be rendered by the camera.  See set_scene().
 */
INLINE const NodePath &Camera::
get_scene() const {
  return _scene;
}

/**
 * Returns the number of display regions associated with the camera.
 */
INLINE size_t Camera::
get_num_display_regions() const {
  return _display_regions.size();
}

/**
 * Returns the nth display region associated with the camera.
 */
INLINE DisplayRegion *Camera::
get_display_region(size_t n) const {
  nassertr(n < _display_regions.size(), nullptr);
  return _display_regions[n];
}

/**
 * Changes the set of bits that represent the subset of the scene graph the
 * camera will render.
 *
 * During the cull traversal, a node is not visited if none of its draw mask
 * bits intersect with the camera's camera mask bits.  These masks can be used
 * to selectively hide and show different parts of the scene graph from
 * different cameras that are otherwise viewing the same scene.
 */
INLINE void Camera::
set_camera_mask(DrawMask mask) {
  // You shouldn't attempt to use Panda's reserved "overall" bit as a camera
  // mask.
  nassertv((mask & PandaNode::get_overall_bit()).is_zero());
  _camera_mask = mask;
}

/**
 * Returns the set of bits that represent the subset of the scene graph the
 * camera will render.  See set_camera_mask().
 */
INLINE DrawMask Camera::
get_camera_mask() const {
  return _camera_mask;
}

/**
 * Specifies the point from which the culling operations are performed.
 * Normally, this is the same as the camera, and that is the default if this
 * is not specified; but it may sometimes be useful to perform the culling
 * from some other viewpoint, particularly when you are debugging the culling
 * itself.
 */
INLINE void Camera::
set_cull_center(const NodePath &cull_center) {
  _cull_center = cull_center;
}

/**
 * Returns the point from which the culling operations will be performed, if
 * it was set by set_cull_center(), or the empty NodePath otherwise.
 */
INLINE const NodePath &Camera::
get_cull_center() const {
  return _cull_center;
}

/**
 * Specifies the bounding volume that should be used to perform culling from
 * this camera.  Normally, this is the bounding volume returned from the
 * active lens' make_bounds() call, but you may override this to specify a
 * custom volume if you require.  The specified bounding volume will be
 * understood to be in the coordinate space of the get_cull_center() node.
 */
INLINE void Camera::
set_cull_bounds(BoundingVolume *cull_bounds) {
  _cull_bounds = cull_bounds;
}

/**
 * Returns the custom cull volume that was set by set_cull_bounds(), if any,
 * or NULL if no custom cull volume was set.
 */
INLINE BoundingVolume *Camera::
get_cull_bounds() const {
  return _cull_bounds;
}

/**
 * Specifies the point from which the LOD distances are measured.  Normally,
 * this is the same as the camera, and that is the default if this is not
 * specified; but it may sometimes be useful to perform the distance test from
 * some other viewpoint.  This may be used, for instance, to reduce LOD
 * popping when the camera rotates in a small circle about an avatar.
 */
INLINE void Camera::
set_lod_center(const NodePath &lod_center) {
  _lod_center = lod_center;
}

/**
 * Returns the point from which the LOD distances will be measured, if it was
 * set by set_lod_center(), or the empty NodePath otherwise.
 */
INLINE const NodePath &Camera::
get_lod_center() const {
  return _lod_center;
}

/**
 * Sets the initial state which is applied to all nodes in the scene, as if it
 * were set at the top of the scene graph.
 */
INLINE void Camera::
set_initial_state(const RenderState *state) {
  _initial_state = state;
}

/**
 * Returns the initial state as set by a previous call to set_initial_state().
 */
INLINE CPT(RenderState) Camera::
get_initial_state() const {
  return _initial_state;
}

/**
 * Sets the tag key which, when encountered as a tag on nodes in the scene
 * graph, causes this Camera to apply an arbitrary state transition based on
 * the value of the tag (as specified to set_tag_state()).
 */
INLINE void Camera::
set_tag_state_key(const std::string &tag_state_key) {
  _tag_state_key = tag_state_key;
}

/**
 * Returns the tag key as set by a previous call to set_tag_state_key().
 */
INLINE const std::string &Camera::
get_tag_state_key() const {
  return _tag_state_key;
}

/**
 * Returns the multiplier for LOD distances.
 */
INLINE PN_stdfloat Camera::
get_lod_scale() const {
  return _lod_scale;
}

/**
 * Sets the multiplier for LOD distances.  This value is multiplied with the
 * LOD scale set on LodNodes.
 */
INLINE void Camera::
set_lod_scale(PN_stdfloat value) {
  _lod_scale = value;
}
