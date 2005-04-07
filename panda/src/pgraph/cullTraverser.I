// Filename: cullTraverser.I
// Created by:  drose (23Feb02)
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
//     Function: CullTraverser::get_gsg
//       Access: Public
//  Description: Returns the GraphicsStateGuardian in effect.
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardianBase *CullTraverser::
get_gsg() const {
  return _gsg;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_scene
//       Access: Public
//  Description: Sets the SceneSetup object that indicates the initial
//               camera position, etc.  This must be called before
//               traversal begins.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
set_scene(SceneSetup *scene_setup) {
  _scene_setup = scene_setup;
  _initial_state = scene_setup->get_initial_state();

  const Camera *camera = scene_setup->get_camera_node();
  _tag_state_key = camera->get_tag_state_key();
  _has_tag_state_key = !_tag_state_key.empty();
  _camera_mask = camera->get_camera_mask();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_scene
//       Access: Public
//  Description: Returns the SceneSetup object.
////////////////////////////////////////////////////////////////////
INLINE SceneSetup *CullTraverser::
get_scene() const {
  return _scene_setup;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::has_tag_state_key
//       Access: Public
//  Description: Returns true if a nonempty tag state key has been
//               specified for the scene's camera, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CullTraverser::
has_tag_state_key() const {
  return _has_tag_state_key;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_tag_state_key
//       Access: Public
//  Description: Returns the tag state key that has been specified for
//               the scene's camera, if any.
////////////////////////////////////////////////////////////////////
INLINE const string &CullTraverser::
get_tag_state_key() const {
  return _tag_state_key;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_camera_transform
//       Access: Public
//  Description: Returns the position of the camera relative to the
//               starting node, without any compensating
//               coordinate-system transforms that might have been
//               introduced for the purposes of rendering.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CullTraverser::
get_camera_transform() const {
  return _scene_setup->get_camera_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_render_transform
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
INLINE const TransformState *CullTraverser::
get_render_transform() const {
  return _scene_setup->get_render_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_initial_state
//       Access: Public
//  Description: Sets the initial RenderState at the top of the scene
//               graph we are traversing.  If this is not set, the
//               default is the empty state.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
set_initial_state(const RenderState *initial_state) {
  _initial_state = initial_state;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_initial_state
//       Access: Public
//  Description: Returns the initial RenderState at the top of the
//               scene graph we are traversing, or the empty state if
//               the initial state was never set.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *CullTraverser::
get_initial_state() const {
  return _initial_state;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_depth_offset_decals
//       Access: Public
//  Description: Sets the depth_offset_decals flag.  If this is true,
//               decals will be rendered using DepthOffsetAttribs;
//               otherwise, decals will be rendered with a more
//               expensive three-pass system.  This is normally set
//               from the corresponding flag in the GSG.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
set_depth_offset_decals(bool flag) {
  _depth_offset_decals = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_depth_offset_decals
//       Access: Public
//  Description: Returns the depth_offset_decals flag.  See
//               set_depth_offset_decals().
////////////////////////////////////////////////////////////////////
INLINE bool CullTraverser::
get_depth_offset_decals() const {
  return _depth_offset_decals;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_camera_mask
//       Access: Public
//  Description: Specifies the visibility mask from the camera viewing
//               the scene.  Any nodes that do not have at least some
//               bits in common with this mask will not be drawn.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
set_camera_mask(const DrawMask &camera_mask) {
  _camera_mask = camera_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_camera_mask
//       Access: Public
//  Description: Returns the visibility mask from the camera viewing
//               the scene.
////////////////////////////////////////////////////////////////////
INLINE const DrawMask &CullTraverser::
get_camera_mask() const {
  return _camera_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_view_frustum
//       Access: Public
//  Description: Specifies the bounding volume that corresponds to the
//               view frustum.  Any primitives that fall entirely
//               outside of this volume are not drawn.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
set_view_frustum(GeometricBoundingVolume *view_frustum) {
  _view_frustum = view_frustum;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_view_frustum
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
INLINE GeometricBoundingVolume *CullTraverser::
get_view_frustum() const {
  return _view_frustum;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_guard_band
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
INLINE void CullTraverser::
set_guard_band(GeometricBoundingVolume *guard_band) {
  _guard_band = guard_band;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_guard_band
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
INLINE GeometricBoundingVolume *CullTraverser::
get_guard_band() const {
  return _guard_band;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_cull_handler
//       Access: Public
//  Description: Specifies the object that will receive the culled
//               Geoms.  This must be set before calling traverse().
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
set_cull_handler(CullHandler *cull_handler) {
  _cull_handler = cull_handler;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_cull_handler
//       Access: Public
//  Description: Returns the object that will receive the culled
//               Geoms.
////////////////////////////////////////////////////////////////////
INLINE CullHandler *CullTraverser::
get_cull_handler() const {
  return _cull_handler;
}
////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_portal_clipper
//       Access: Public
//  Description: Specifies _portal_clipper object pointer that 
//               subsequent traverse() or traverse_below may use.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
set_portal_clipper(PortalClipper *portal_clipper) {
  _portal_clipper = portal_clipper;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_portal_clipper
//       Access: Public
//  Description: Returns the _portal_clipper pointer
////////////////////////////////////////////////////////////////////

INLINE PortalClipper *CullTraverser::
get_portal_clipper() const {
  return _portal_clipper;
}
