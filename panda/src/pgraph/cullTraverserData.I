// Filename: cullTraverserData.I
// Created by:  drose (06Mar02)
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
//     Function: CullTraverserData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CullTraverserData::
CullTraverserData(const NodePath &start,
                  const TransformState *net_transform,
                  const RenderState *state,
                  GeometricBoundingVolume *view_frustum,
                  Thread *current_thread) :
  _node_path(start),
  _node_reader(start.node(), current_thread),
  _net_transform(net_transform),
  _state(state),
  _view_frustum(view_frustum),
  _cull_planes(CullPlanes::make_empty()),
  _draw_mask(DrawMask::all_on()),
  _portal_depth(0)
{
  // Only update the bounding volume if we're going to end up needing it.
  bool check_bounds = (view_frustum != (GeometricBoundingVolume *)NULL);
  _node_reader.check_cached(check_bounds);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CullTraverserData::
CullTraverserData(const CullTraverserData &copy) :
  _node_path(copy._node_path),
  _node_reader(copy._node_reader),
  _net_transform(copy._net_transform),
  _state(copy._state),
  _view_frustum(copy._view_frustum),
  _cull_planes(copy._cull_planes),
  _draw_mask(copy._draw_mask),
  _portal_depth(copy._portal_depth)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CullTraverserData::
operator = (const CullTraverserData &copy) {
  _node_path = copy._node_path;
  _node_reader = copy._node_reader;
  _net_transform = copy._net_transform;
  _state = copy._state;
  _view_frustum = copy._view_frustum;
  _cull_planes = copy._cull_planes;
  _draw_mask = copy._draw_mask;
  _portal_depth = copy._portal_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::Constructor
//       Access: Public
//  Description: This constructor creates a CullTraverserData object
//               that reflects the next node down in the traversal.
////////////////////////////////////////////////////////////////////
INLINE CullTraverserData::
CullTraverserData(const CullTraverserData &parent, PandaNode *child) :
  _node_path(parent._node_path, child),
  _node_reader(child, parent._node_reader.get_current_thread()),
  _net_transform(parent._net_transform),
  _state(parent._state),
  _view_frustum(parent._view_frustum),
  _cull_planes(parent._cull_planes),
  _draw_mask(parent._draw_mask),
  _portal_depth(parent._portal_depth)
{
  // Only update the bounding volume if we're going to end up needing it.
  bool check_bounds = !_cull_planes->is_empty() ||
                    (_view_frustum != (GeometricBoundingVolume *)NULL);
  _node_reader.check_cached(check_bounds);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CullTraverserData::
~CullTraverserData() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::node
//       Access: Published
//  Description: Returns the node traversed to so far.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *CullTraverserData::
node() const {
  return _node_path.node();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::node_reader
//       Access: Public
//  Description: Returns the PipelineReader for the node traversed to
//               so far.
////////////////////////////////////////////////////////////////////
INLINE PandaNodePipelineReader *CullTraverserData::
node_reader() {
  return &_node_reader;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::node_reader
//       Access: Public
//  Description: Returns the PipelineReader for the node traversed to
//               so far.
////////////////////////////////////////////////////////////////////
INLINE const PandaNodePipelineReader *CullTraverserData::
node_reader() const {
  return &_node_reader;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::get_modelview_transform
//       Access: Published
//  Description: Returns the modelview transform: the relative
//               transform from the camera to the model.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) CullTraverserData::
get_modelview_transform(const CullTraverser *trav) const {
  return trav->get_world_transform()->compose(_net_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::get_internal_transform
//       Access: Published
//  Description: Returns the internal transform: the modelview
//               transform in the GSG's internal coordinate system.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) CullTraverserData::
get_internal_transform(const CullTraverser *trav) const {
  return trav->get_scene()->get_cs_world_transform()->compose(_net_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::get_net_transform
//       Access: Published
//  Description: Returns the net transform: the relative transform
//               from root of the scene graph to the current node.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CullTraverserData::
get_net_transform(const CullTraverser *) const {
  return _net_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::is_in_view
//       Access: Published
//  Description: Returns true if the current node is within the view
//               frustum, false otherwise.  If the node's bounding
//               volume falls completely within the view frustum, this
//               will also reset the view frustum pointer, saving some
//               work for future nodes.
////////////////////////////////////////////////////////////////////
INLINE bool CullTraverserData::
is_in_view(const DrawMask &camera_mask) {
  if (_node_reader.get_transform()->is_invalid()) {
    // If the transform is invalid, forget it.
    return false;
  }

  if (!_node_reader.compare_draw_mask(_draw_mask, camera_mask)) {
    // If there are no draw bits in common with the camera, the node
    // is out.
    return false;
  }

  if (_view_frustum == (GeometricBoundingVolume *)NULL &&
      _cull_planes->is_empty()) {
    // If the transform is valid, but we don't have a frustum or any
    // clip planes or occluders, it's always in.
    return true;
  }

  // Otherwise, compare the bounding volume to the frustum.
  return is_in_view_impl();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::is_this_node_hidden
//       Access: Published
//  Description: Returns true if this particular node is hidden, even
//               though we might be traversing past this node to find
//               a child node that has had show_through() called for
//               it.  If this returns true, the node should not be
//               rendered.
////////////////////////////////////////////////////////////////////
INLINE bool CullTraverserData::
is_this_node_hidden(const DrawMask &camera_mask) const {
  return (_draw_mask & PandaNode::get_overall_bit()).is_zero() ||
    (_draw_mask & camera_mask).is_zero();
}
