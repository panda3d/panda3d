// Filename: cullTraverserData.I
// Created by:  drose (06Mar02)
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
//     Function: CullTraverserData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullTraverserData::
CullTraverserData(const qpNodePath &start,
                  const TransformState *render_transform,
                  const TransformState *net_transform,
                  const RenderState *state,
                  GeometricBoundingVolume *view_frustum,
                  GeometricBoundingVolume *guard_band) :
  _node_path(start),
  _render_transform(render_transform),
  _net_transform(net_transform),
  _state(state),
  _view_frustum(view_frustum),
  _guard_band(guard_band)
{
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
  _render_transform(parent._render_transform),
  _net_transform(parent._net_transform),
  _state(parent._state),
  _view_frustum(parent._view_frustum),
  _guard_band(parent._guard_band)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::Copy Constructor
//       Access: Private
//  Description: Do not copy CullTraverserData objects.
////////////////////////////////////////////////////////////////////
INLINE CullTraverserData::
CullTraverserData(const CullTraverserData &copy) :
  _node_path(qpNodePath())
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::Copy Assignment Operator
//       Access: Private
//  Description: Do not copy CullTraverserData objects.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverserData::
operator = (const CullTraverserData &copy) {
  nassertv(false);
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

  if ((node()->get_draw_mask() & camera_mask).is_zero()) {
    // If there are no draw bits in common with the camera, the node
    // is out.
    return false;
  }

  if (_view_frustum == (GeometricBoundingVolume *)NULL) {
    // If the transform is valid, but we don't have a frustum, it's
    // always in.
    return true;
  }

  // Otherwise, compare the bounding volume to the frustum.
  return is_in_view_impl();
}
