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
//     Function: qpCullTraverser::get_initial_state
//       Access: Public
//  Description: Returns the initial RenderState at the top of the
//               scene graph we are traversing, or the empty state if
//               the initial state was never set.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *qpCullTraverser::
get_initial_state() const {
  return _initial_state;
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
//     Function: qpCullTraverser::get_camera_mask
//       Access: Public
//  Description: Returns the visibility mask from the camera viewing
//               the scene.
////////////////////////////////////////////////////////////////////
INLINE const DrawMask &qpCullTraverser::
get_camera_mask() const {
  return _camera_mask;
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
//     Function: qpCullTraverser::get_camera_transform
//       Access: Public
//  Description: Returns the position of the camera relative to the
//               starting node, without any compensating
//               coordinate-system transforms that might have been
//               introduced for the purposes of rendering.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *qpCullTraverser::
get_camera_transform() const {
  return _camera_transform;
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
//     Function: qpCullTraverser::get_render_transform
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
INLINE const TransformState *qpCullTraverser::
get_render_transform() const {
  return _render_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_view_frustum
//       Access: Public
//  Description: Specifies the bounding volume that corresponds to the
//               view frustum.  Any primitives that fall entirely
//               outside of this volume are not drawn.
////////////////////////////////////////////////////////////////////
INLINE void qpCullTraverser::
set_view_frustum(GeometricBoundingVolume *view_frustum) {
  _view_frustum = view_frustum;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::get_view_frustum
//       Access: Public
//  Description: Returns the bounding volume that corresponds to the
//               view frustum, or NULL if the view frustum is not in
//               use or has not been set.
//
//               Note that the view frustum returned here is always in
//               the coordinate space of the starting node, not the
//               current node, even if it is sampled during a
//               traversal.  To get the view frustum in the current
//               node's coordinate space, check in the current
//               CullTraverserData.
////////////////////////////////////////////////////////////////////
INLINE GeometricBoundingVolume *qpCullTraverser::
get_view_frustum() const {
  return _view_frustum;
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
//     Function: qpCullTraverser::get_guard_band
//       Access: Public
//  Description: Returns the bounding volume that corresponds to the
//               guard band, or NULL if the guard band is not in
//               use or has not been set.
//
//               Note that the guard band returned here is always in
//               the coordinate space of the starting node, not the
//               current node, even if it is sampled during a
//               traversal.  To get the guard band in the current
//               node's coordinate space, check in the current
//               CullTraverserData.
////////////////////////////////////////////////////////////////////
INLINE GeometricBoundingVolume *qpCullTraverser::
get_guard_band() const {
  return _guard_band;
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

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::get_cull_handler
//       Access: Public
//  Description: Returns the object that will receive the culled
//               Geoms.
////////////////////////////////////////////////////////////////////
INLINE CullHandler *qpCullTraverser::
get_cull_handler() const {
  return _cull_handler;
}
