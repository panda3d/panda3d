/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullTraverserData.I
 * @author drose
 * @date 2002-03-06
 */

/**
 *
 */
INLINE CullTraverserData::
CullTraverserData(const NodePath &start,
                  const TransformState *net_transform,
                  CPT(RenderState) state,
                  GeometricBoundingVolume *view_frustum,
                  Thread *current_thread) :
  _next(nullptr),
  _start(start._head),
  _node_reader(start.node(), current_thread),
  _net_transform(net_transform),
  _state(std::move(state)),
  _view_frustum(view_frustum),
  _draw_mask(DrawMask::all_on()),
  _portal_depth(0)
{
}

/**
 * This constructor creates a CullTraverserData object that reflects the next
 * node down in the traversal.
 */
INLINE CullTraverserData::
CullTraverserData(const CullTraverserData &parent, PandaNode *child,
                  const TransformState *net_transform,
                  CPT(RenderState) state,
                  GeometricBoundingVolume *view_frustum) :
  _next(&parent),
#ifdef _DEBUG
  _start(nullptr),
#endif
  _node_reader(child, parent._node_reader.get_current_thread()),
  _net_transform(net_transform),
  _state(std::move(state)),
  _view_frustum(view_frustum),
  _instances(parent._instances),
  _draw_mask(parent._draw_mask),
  _portal_depth(parent._portal_depth)
{
}

/**
 * This constructor creates a CullTraverserData object that reflects the next
 * node down in the traversal.
 */
INLINE CullTraverserData::
CullTraverserData(const CullTraverserData &parent,
                  PandaNodePipelineReader &&node_reader,
                  const TransformState *net_transform,
                  CPT(RenderState) state,
                  GeometricBoundingVolume *view_frustum) :
  _next(&parent),
#ifdef _DEBUG
  _start(nullptr),
#endif
  _node_reader(std::move(node_reader)),
  _net_transform(net_transform),
  _state(std::move(state)),
  _view_frustum(view_frustum),
  _instances(parent._instances),
  _draw_mask(parent._draw_mask),
  _portal_depth(parent._portal_depth)
{
}

/**
 * Returns the node traversed to so far.
 */
INLINE PandaNode *CullTraverserData::
node() const {
  return (PandaNode *)_node_reader.get_node();
}

/**
 * Returns the PipelineReader for the node traversed to so far.
 */
INLINE PandaNodePipelineReader *CullTraverserData::
node_reader() {
  return &_node_reader;
}

/**
 * Returns the PipelineReader for the node traversed to so far.
 */
INLINE const PandaNodePipelineReader *CullTraverserData::
node_reader() const {
  return &_node_reader;
}

/**
 * Constructs and returns an actual NodePath that represents the same path we
 * have just traversed.
 */
INLINE NodePath CullTraverserData::
get_node_path() const {
  NodePath result;
  result._head = r_get_node_path();
  nassertr(result._head != nullptr, NodePath::fail());
  return result;
}

/**
 * Returns the transformed view frustum being used for culling at this level.
 */
INLINE GeometricBoundingVolume *CullTraverserData::
get_view_frustum() const {
  return _view_frustum.p();
}

/**
 * Changes the transformed view frustum being used for culling at this level.
 */
INLINE void CullTraverserData::
set_view_frustum(PT(GeometricBoundingVolume) view_frustum) {
  if (_view_frustum == nullptr && view_frustum != nullptr && _cull_planes == nullptr) {
    // In this case we will need up-to-date bounds but we may not have them.
    _node_reader.check_cached(true);
  }
  _view_frustum = std::move(view_frustum);
}

/**
 * Returns the modelview transform: the relative transform from the camera to
 * the model.
 */
INLINE CPT(TransformState) CullTraverserData::
get_modelview_transform(const CullTraverser *trav) const {
  return trav->get_world_transform()->compose(_net_transform);
}

/**
 * Returns the internal transform: the modelview transform in the GSG's
 * internal coordinate system.
 */
INLINE CPT(TransformState) CullTraverserData::
get_internal_transform(const CullTraverser *trav) const {
  const TransformState *cs_world_transform = trav->get_scene()->get_cs_world_transform();
  return cs_world_transform->compose(_net_transform);
}

/**
 * Returns the net transform: the relative transform from root of the scene
 * graph to the current node.
 */
INLINE const TransformState *CullTraverserData::
get_net_transform(const CullTraverser *) const {
  return _net_transform;
}

/**
 * Returns intersection flags if the current node may be within the view
 * frustum, 0 otherwise.
 */
INLINE int CullTraverserData::
is_in_view(const DrawMask &camera_mask) const {
  bool check_bounds = (_view_frustum != nullptr || _cull_planes != nullptr);
  _node_reader.check_cached(check_bounds);

  if (!_node_reader.compare_draw_mask(_draw_mask, camera_mask)) {
    // If there are no draw bits in common with the camera, the node is out.
    return BoundingVolume::IF_no_intersection;
  }

  if (_node_reader.get_transform()->is_invalid()) {
    // If the transform is invalid, forget it.
    return BoundingVolume::IF_no_intersection;
  }

  if (_view_frustum != nullptr) {
    const GeometricBoundingVolume *node_gbv = _node_reader.get_bounds()->as_geometric_bounding_volume();
    nassertr(node_gbv != nullptr, BoundingVolume::IF_no_intersection);

    return _view_frustum->contains(node_gbv);
  }

  return BoundingVolume::IF_possible;
}

/**
 * Returns intersection flags if the given child of the current node may be
 * within the view frustum, 0 otherwise.
 */
INLINE int CullTraverserData::
is_child_in_view(const PandaNodePipelineReader &node_reader, const DrawMask &camera_mask) const {
  bool check_bounds = (_view_frustum != nullptr || _cull_planes != nullptr);
  node_reader.check_cached(check_bounds);

  if (!node_reader.compare_draw_mask(_draw_mask, camera_mask)) {
    // If there are no draw bits in common with the camera, the node is out.
    return BoundingVolume::IF_no_intersection;
  }

  if (_node_reader.get_transform()->is_invalid()) {
    // If the transform is invalid, forget it.
    return BoundingVolume::IF_no_intersection;
  }

  if (_view_frustum != nullptr) {
    const GeometricBoundingVolume *node_gbv = node_reader.get_bounds()->as_geometric_bounding_volume();
    nassertr(node_gbv != nullptr, BoundingVolume::IF_no_intersection);

    return _view_frustum->contains(node_gbv);
  }
  return BoundingVolume::IF_possible;
}

/**
 * Returns intersection flags if the given child of the current node may be
 * within the view frustum, 0 otherwise.
 */
INLINE int CullTraverserData::
is_child_in_view(const PandaNode::DownConnection &child, const DrawMask &camera_mask) const {
  // No need to call check_cached here, since we presumably already called it
  // on the parent.
  if (!child.compare_draw_mask(_draw_mask, camera_mask)) {
    // If there are no draw bits in common with the camera, the node is out.
    return BoundingVolume::IF_no_intersection;
  }

  if (_view_frustum != nullptr) {
    const GeometricBoundingVolume *node_gbv = child.get_bounds();
    nassertr(node_gbv != nullptr, BoundingVolume::IF_no_intersection);

    return _view_frustum->contains(node_gbv);
  }
  return BoundingVolume::IF_possible;
}

/**
 * Returns true if this particular node is hidden, even though we might be
 * traversing past this node to find a child node that has had show_through()
 * called for it.  If this returns true, the node should not be rendered.
 */
INLINE bool CullTraverserData::
is_this_node_hidden(const DrawMask &camera_mask) const {
  return (_draw_mask & PandaNode::get_overall_bit()).is_zero() ||
    (_draw_mask & camera_mask).is_zero();
}
