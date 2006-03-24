// Filename: cullTraverserData.I
// Created by:  drose (06Mar02)
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
//     Function: CullTraverserData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullTraverserData::
CullTraverserData(const NodePath &start,
                  const TransformState *net_transform,
                  const RenderState *state,
                  GeometricBoundingVolume *view_frustum,
                  GeometricBoundingVolume *guard_band) :
  _node_path(start),
  _net_transform(net_transform),
  _state(state),
  _view_frustum(view_frustum),
  _guard_band(guard_band),
  _cull_planes(CullPlanes::make_empty()),
  _draw_mask(DrawMask::all_on())
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullTraverserData::
CullTraverserData(const CullTraverserData &copy) :
  _node_path(copy._node_path),
  _net_transform(copy._net_transform),
  _state(copy._state),
  _view_frustum(copy._view_frustum),
  _guard_band(copy._guard_band),
  _cull_planes(copy._cull_planes),
  _draw_mask(copy._draw_mask)
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
  _net_transform = copy._net_transform;
  _state = copy._state;
  _view_frustum = copy._view_frustum;
  _guard_band = copy._guard_band;
  _cull_planes = copy._cull_planes;
  _draw_mask = copy._draw_mask;
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
  _net_transform(parent._net_transform),
  _state(parent._state),
  _view_frustum(parent._view_frustum),
  _guard_band(parent._guard_band),
  _cull_planes(parent._cull_planes),
  _draw_mask(parent._draw_mask)
{
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
//       Access: Public
//  Description: Returns the node traversed to so far.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *CullTraverserData::
node() const {
  return _node_path.node();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::get_net_transform
//       Access: Public
//  Description: Returns the net transform: the relative transform
//               from root of the scene graph to the current node.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CullTraverserData::
get_net_transform(const CullTraverser *) const {
  return _net_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::is_in_view
//       Access: Public
//  Description: Returns true if the current node is within the view
//               frustum, false otherwise.  If the node's bounding
//               volume falls completely within the view frustum, this
//               will also reset the view frustum pointer, saving some
//               work for future nodes.
////////////////////////////////////////////////////////////////////
INLINE bool CullTraverserData::
is_in_view(const DrawMask &camera_mask) {
  if (node()->get_transform()->is_invalid()) {
    // If the transform is invalid, forget it.
    return false;
  }

  if (!node()->compare_draw_mask(_draw_mask, camera_mask)) {
    // If there are no draw bits in common with the camera, the node
    // is out.
    return false;
  }

  if (_view_frustum == (GeometricBoundingVolume *)NULL &&
      _cull_planes->is_empty()) {
    // If the transform is valid, but we don't have a frustum or any
    // clip planes, it's always in.
    return true;
  }

  // Otherwise, compare the bounding volume to the frustum.
  return is_in_view_impl();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::is_this_node_hidden
//       Access: Public
//  Description: Returns true if this particular node is hidden, even
//               though we might be traversing past this node to find
//               a child node that has had show_through() called for
//               it.  If this returns true, the node should not be
//               rendered.
////////////////////////////////////////////////////////////////////
INLINE bool CullTraverserData::
is_this_node_hidden(const CullTraverser *trav) const {
  return (_draw_mask & PandaNode::get_overall_bit()).is_zero() ||
    (_draw_mask & trav->get_camera_mask()).is_zero();
}
