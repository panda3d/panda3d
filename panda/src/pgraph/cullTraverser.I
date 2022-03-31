/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullTraverser.I
 * @author drose
 * @date 2002-02-23
 */

/**
 * Returns the GraphicsStateGuardian in effect.
 */
INLINE GraphicsStateGuardianBase *CullTraverser::
get_gsg() const {
  return _gsg;
}

/**
 * Returns the currently-executing thread object, as passed to the
 * CullTraverser constructor.
 */
INLINE Thread *CullTraverser::
get_current_thread() const {
  return _current_thread;
}

/**
 * Returns the SceneSetup object.
 */
INLINE SceneSetup *CullTraverser::
get_scene() const {
  return _scene_setup;
}

/**
 * Returns true if a nonempty tag state key has been specified for the scene's
 * camera, false otherwise.
 */
INLINE bool CullTraverser::
has_tag_state_key() const {
  return _has_tag_state_key;
}

/**
 * Returns the tag state key that has been specified for the scene's camera,
 * if any.
 */
INLINE const std::string &CullTraverser::
get_tag_state_key() const {
  return _tag_state_key;
}

/**
 * Returns the position of the camera relative to the starting node.
 */
INLINE const TransformState *CullTraverser::
get_camera_transform() const {
  return _scene_setup->get_camera_transform();
}

/**
 * Returns the position of the starting node relative to the camera.  This is
 * the inverse of the camera transform.
 *
 * Note that this value is always the position of the starting node, not the
 * current node, even if it is sampled during a traversal.  To get the
 * transform of the current node use
 * CullTraverserData::get_modelview_transform().
 */
INLINE const TransformState *CullTraverser::
get_world_transform() const {
  return _scene_setup->get_world_transform();
}

/**
 * Returns the initial RenderState at the top of the scene graph we are
 * traversing, or the empty state if the initial state was never set.
 */
INLINE const RenderState *CullTraverser::
get_initial_state() const {
  return _initial_state;
}

/**
 * Returns true, as depth offsets are the only way that we implement decals
 * nowadays.
 */
INLINE bool CullTraverser::
get_depth_offset_decals() const {
  return true;
}

/**
 * Changes the visibility mask for the camera viewing the scene.  This is
 * normally set automatically at the time setup_scene() is called; you should
 * change this only if you want to render some set of objects different from
 * what the camera normally would draw.
 */
INLINE void CullTraverser::
set_camera_mask(const DrawMask &camera_mask) {
  _camera_mask = camera_mask;
}

/**
 * Returns the visibility mask from the camera viewing the scene.
 */
INLINE const DrawMask &CullTraverser::
get_camera_mask() const {
  return _camera_mask;
}

/**
 * Specifies the bounding volume that corresponds to the view frustum.  Any
 * primitives that fall entirely outside of this volume are not drawn.
 *
 * Nowadays, this gets set automatically by set_scene().
 */
INLINE void CullTraverser::
set_view_frustum(GeometricBoundingVolume *view_frustum) {
  _view_frustum = view_frustum;
}

/**
 * Returns the bounding volume that corresponds to the view frustum, or NULL
 * if the view frustum is not in use or has not been set.
 *
 * Note that the view frustum returned here is always in the coordinate space
 * of the starting node, not the current node, even if it is sampled during a
 * traversal.  To get the view frustum in the current node's coordinate space,
 * check in the current CullTraverserData.
 */
INLINE GeometricBoundingVolume *CullTraverser::
get_view_frustum() const {
  return _view_frustum;
}

/**
 * Specifies the object that will receive the culled Geoms.  This must be set
 * before calling traverse().
 */
INLINE void CullTraverser::
set_cull_handler(CullHandler *cull_handler) {
  _cull_handler = cull_handler;
}

/**
 * Returns the object that will receive the culled Geoms.
 */
INLINE CullHandler *CullTraverser::
get_cull_handler() const {
  return _cull_handler;
}
/**
 * Specifies _portal_clipper object pointer that subsequent traverse() or
 * traverse_below may use.
 */
INLINE void CullTraverser::
set_portal_clipper(PortalClipper *portal_clipper) {
  _portal_clipper = portal_clipper;
}

/**
 * Returns the _portal_clipper pointer
 */
INLINE PortalClipper *CullTraverser::
get_portal_clipper() const {
  return _portal_clipper;
}

/**
 * Returns true if the cull traversal is effectively in incomplete_render
 * state, considering both the GSG's incomplete_render and the current
 * DisplayRegion's incomplete_render flags.  This returns the flag during the
 * cull traversal; see GSG::get_effective_incomplete_render() for this same
 * flag during the draw traversal.
 */
INLINE bool CullTraverser::
get_effective_incomplete_render() const {
  return _effective_incomplete_render;
}

/**
 * Returns true if fake view frustum culling is active.
 */
INLINE bool CullTraverser::
get_fake_view_frustum_cull() const {
  return _fake_view_frustum_cull;
}

/**
 * Flushes the PStatCollectors used during traversal.
 */
INLINE void CullTraverser::
flush_level() {
  _nodes_pcollector.flush_level();
  _geom_nodes_pcollector.flush_level();
  _pgui_nodes_pcollector.flush_level();
  _geoms_pcollector.flush_level();
  _geoms_occluded_pcollector.flush_level();
}

/**
 * Calls traverse_down on each child.
 */
INLINE void CullTraverser::
traverse_below(CullTraverserData &data) {
  PandaNodePipelineReader *node_reader = data.node_reader();
  PandaNode::Children children = node_reader->get_children();
  node_reader->release();
  int num_children = children.get_num_children();
  for (int i = 0; i < num_children; ++i) {
    traverse_down(data, children.get_child_connection(i), data._state);
  }
}

/**
 * Traverses a child of the given node/data.
 */
INLINE void CullTraverser::
traverse_down(const CullTraverserData &data, PandaNode *node) {
  traverse_down(data, node, data._net_transform, data._state);
}

/**
 * Traverses down into the given node/data, overriding the current transform
 * and state.  Note that the state and transform on the given node are still
 * applied.
 */
INLINE void CullTraverser::
traverse_down(const CullTraverserData &data, PandaNode *node,
               const TransformState *net_transform, const RenderState *state) {
  PandaNodePipelineReader node_reader(node, data._node_reader.get_current_thread());

  int result = data.is_child_in_view(node_reader, _camera_mask);
  if (result == BoundingVolume::IF_no_intersection) {
#ifdef NDEBUG
    return;
#else
    if (UNLIKELY(_fake_view_frustum_cull)) {
      do_fake_cull(data, node, net_transform, state);
    }
    return;
#endif
  }

  GeometricBoundingVolume *view_frustum = nullptr;
  if ((result & BoundingVolume::IF_all) == 0 && !node_reader.is_final()) {
    view_frustum = data._view_frustum;
  }

  CullTraverserData next_data(data, std::move(node_reader), net_transform, state, view_frustum);
  if (data._cull_planes != nullptr) {
    const GeometricBoundingVolume *node_gbv = node_reader.get_bounds()->as_geometric_bounding_volume();
    if (!next_data.apply_cull_planes(data._cull_planes, node_gbv)) {
      return;
    }
  }

  do_traverse(next_data);
}

/**
 * Traverses down into the given node, as though it were a child node of the
 * current node.
 */
INLINE void CullTraverser::
traverse_down(const CullTraverserData &data, const PandaNode::DownConnection &child) {
  traverse_down(data, child, data._state);
}

/**
 * Traverses down into the given node, as though it were a child node of the
 * current node, except that the current state is ignored and replaced with the
 * given state.  The state on the child node is still applied.
 */
INLINE void CullTraverser::
traverse_down(const CullTraverserData &data, const PandaNode::DownConnection &child, const RenderState *state) {
  int result = data.is_child_in_view(child, _camera_mask);
  if (result == BoundingVolume::IF_no_intersection) {
#ifdef NDEBUG
    return;
#else
    if (UNLIKELY(_fake_view_frustum_cull)) {
      do_fake_cull(data, child.get_child(), data._net_transform, state);
    }
    return;
#endif
  }

  PandaNodePipelineReader node_reader(child.get_child(), data._node_reader.get_current_thread());

  GeometricBoundingVolume *view_frustum = nullptr;
  if ((result & BoundingVolume::IF_all) == 0 && !node_reader.is_final()) {
    view_frustum = data._view_frustum;
  }

  CullTraverserData next_data(data, std::move(node_reader), data._net_transform, state, view_frustum);

  if (data._cull_planes != nullptr &&
      !next_data.apply_cull_planes(data._cull_planes, child.get_bounds())) {
    return;
  }

  do_traverse(next_data);
}
