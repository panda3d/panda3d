// Filename: qpcullTraverser.I
// Created by:  drose (23Feb02)
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
//     Function: qpCullTraverser::set_initial_state
//       Access: Public
//  Description: Sets the initial RenderState at the top of the scene
//               graph we are traversing.  If this is not set, the
//               default is the empty state.
////////////////////////////////////////////////////////////////////
INLINE void qpCullTraverser::
set_initial_state(const RenderState *initial_state) {
  _initial_state = initial_state;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_camera_mask
//       Access: Public
//  Description: Specifies the visibility mask from the camera viewing
//               the scene.  Any nodes that do not have at least some
//               bits in common with this mask will not be drawn.
////////////////////////////////////////////////////////////////////
INLINE void qpCullTraverser::
set_camera_mask(const DrawMask &camera_mask) {
  _camera_mask = camera_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_camera_transform
//       Access: Public
//  Description: Specifies the position of the camera relative to the
//               starting node, without any compensating
//               coordinate-system transforms that might have been
//               introduced for the purposes of rendering.
////////////////////////////////////////////////////////////////////
INLINE void qpCullTraverser::
set_camera_transform(const TransformState *camera_transform) {
  _camera_transform = camera_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_render_transform
//       Access: Public
//  Description: Specifies the position of the starting node relative
//               to the camera, pretransformed as appropriate for
//               rendering.
////////////////////////////////////////////////////////////////////
INLINE void qpCullTraverser::
set_render_transform(const TransformState *render_transform) {
  _render_transform = render_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_view_frustum
//       Access: Public
//  Description: Specifies the bounding volume that corresponds to the
//               viewing frustum.  Any primitives that fall entirely
//               outside of this volume are not drawn.
////////////////////////////////////////////////////////////////////
INLINE void qpCullTraverser::
set_view_frustum(GeometricBoundingVolume *view_frustum) {
  _view_frustum = view_frustum;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_guard_band
//       Access: Public
//  Description: Specifies the bounding volume to use for detecting
//               guard band clipping.  This is a render optimization
//               for certain cards that support this feature; the
//               guard band is a 2-d area than the frame buffer.
//               If a primitive will appear entirely within the guard
//               band after perspective transform, it may be drawn
//               correctly with clipping disabled, for a small
//               performance gain.
//
//               This is the bounding volume that corresponds to the
//               2-d guard band.  If a primitive is entirely within
//               this area, clipping will be disabled on the GSG.
////////////////////////////////////////////////////////////////////
INLINE void qpCullTraverser::
set_guard_band(GeometricBoundingVolume *guard_band) {
  _guard_band = guard_band;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_cull_handler
//       Access: Public
//  Description: Specifies the object that will receive the culled
//               Geoms.  This must be set before calling traverse().
////////////////////////////////////////////////////////////////////
INLINE void qpCullTraverser::
set_cull_handler(CullHandler *cull_handler) {
  _cull_handler = cull_handler;
}
