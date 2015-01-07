// Filename: cullTraverser.I
// Created by:  drose (23Feb02)
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
//     Function: CullTraverser::get_gsg
//       Access: Published
//  Description: Returns the GraphicsStateGuardian in effect.
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardianBase *CullTraverser::
get_gsg() const {
  return _gsg;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_current_thread
//       Access: Published
//  Description: Returns the currently-executing thread object, as
//               passed to the CullTraverser constructor.
////////////////////////////////////////////////////////////////////
INLINE Thread *CullTraverser::
get_current_thread() const {
  return _current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_scene
//       Access: Published
//  Description: Returns the SceneSetup object.
////////////////////////////////////////////////////////////////////
INLINE SceneSetup *CullTraverser::
get_scene() const {
  return _scene_setup;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::has_tag_state_key
//       Access: Published
//  Description: Returns true if a nonempty tag state key has been
//               specified for the scene's camera, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CullTraverser::
has_tag_state_key() const {
  return _has_tag_state_key;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_tag_state_key
//       Access: Published
//  Description: Returns the tag state key that has been specified for
//               the scene's camera, if any.
////////////////////////////////////////////////////////////////////
INLINE const string &CullTraverser::
get_tag_state_key() const {
  return _tag_state_key;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_camera_transform
//       Access: Published
//  Description: Returns the position of the camera relative to the
//               starting node.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CullTraverser::
get_camera_transform() const {
  return _scene_setup->get_camera_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_world_transform
//       Access: Published
//  Description: Returns the position of the starting node relative
//               to the camera.  This is the inverse of the camera
//               transform.
//
//               Note that this value is always the position of the
//               starting node, not the current node, even if it is
//               sampled during a traversal.  To get the transform of
//               the current node use
//               CullTraverserData::get_modelview_transform().
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CullTraverser::
get_world_transform() const {
  return _scene_setup->get_world_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_initial_state
//       Access: Published
//  Description: Returns the initial RenderState at the top of the
//               scene graph we are traversing, or the empty state if
//               the initial state was never set.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *CullTraverser::
get_initial_state() const {
  return _initial_state;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_depth_offset_decals
//       Access: Published
//  Description: Returns true, as depth offsets are the only way
//               that we implement decals nowadays.
////////////////////////////////////////////////////////////////////
INLINE bool CullTraverser::
get_depth_offset_decals() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_camera_mask
//       Access: Published
//  Description: Changes the visibility mask for the camera viewing
//               the scene.  This is normally set automatically
//               at the time setup_scene() is called; you should
//               change this only if you want to render some set of
//               objects different from what the camera normally would
//               draw.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
set_camera_mask(const DrawMask &camera_mask) {
  _camera_mask = camera_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_camera_mask
//       Access: Published
//  Description: Returns the visibility mask from the camera viewing
//               the scene.
////////////////////////////////////////////////////////////////////
INLINE const DrawMask &CullTraverser::
get_camera_mask() const {
  return _camera_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_view_frustum
//       Access: Published
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
//       Access: Published
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
//     Function: CullTraverser::set_cull_handler
//       Access: Published
//  Description: Specifies the object that will receive the culled
//               Geoms.  This must be set before calling traverse().
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
set_cull_handler(CullHandler *cull_handler) {
  _cull_handler = cull_handler;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_cull_handler
//       Access: Published
//  Description: Returns the object that will receive the culled
//               Geoms.
////////////////////////////////////////////////////////////////////
INLINE CullHandler *CullTraverser::
get_cull_handler() const {
  return _cull_handler;
}
////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::set_portal_clipper
//       Access: Published
//  Description: Specifies _portal_clipper object pointer that
//               subsequent traverse() or traverse_below may use.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
set_portal_clipper(PortalClipper *portal_clipper) {
  _portal_clipper = portal_clipper;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_portal_clipper
//       Access: Published
//  Description: Returns the _portal_clipper pointer
////////////////////////////////////////////////////////////////////
INLINE PortalClipper *CullTraverser::
get_portal_clipper() const {
  return _portal_clipper;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_effective_incomplete_render
//       Access: Published
//  Description: Returns true if the cull traversal is effectively in
//               incomplete_render state, considering both the GSG's
//               incomplete_render and the current DisplayRegion's
//               incomplete_render flags.  This returns the flag
//               during the cull traversal; see
//               GSG::get_effective_incomplete_render() for this same
//               flag during the draw traversal.
////////////////////////////////////////////////////////////////////
INLINE bool CullTraverser::
get_effective_incomplete_render() const {
  return _effective_incomplete_render;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::flush_level
//       Access: Published, Static
//  Description: Flushes the PStatCollectors used during traversal.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
flush_level() {
  _nodes_pcollector.flush_level();
  _geom_nodes_pcollector.flush_level();
  _geoms_pcollector.flush_level();
  _geoms_occluded_pcollector.flush_level();
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::do_traverse
//       Access: Protected
//  Description: This is implemented inline to reduce recursion.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
do_traverse(CullTraverserData &data) {
  if (is_in_view(data)) {
    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << "\n" << data._node_path
        << " " << data._draw_mask << "\n";
    }

    PandaNodePipelineReader *node_reader = data.node_reader();
    int fancy_bits = node_reader->get_fancy_bits();

    if ((fancy_bits & (PandaNode::FB_transform |
                       PandaNode::FB_state |
                       PandaNode::FB_effects |
                       PandaNode::FB_tag |
                       PandaNode::FB_draw_mask |
                       PandaNode::FB_cull_callback)) == 0 &&
        data._cull_planes->is_empty()) {
      // Nothing interesting in this node; just move on.

    } else {
      // Something in this node is worth taking a closer look.
      const RenderEffects *node_effects = node_reader->get_effects();
      if (node_effects->has_show_bounds()) {
        // If we should show the bounding volume for this node, make it
        // up now.
        show_bounds(data, node_effects->has_show_tight_bounds());
      }

      data.apply_transform_and_state(this);

      const FogAttrib *fog = (const FogAttrib *)
        node_reader->get_state()->get_attrib(FogAttrib::get_class_slot());

      if (fog != (const FogAttrib *)NULL && fog->get_fog() != (Fog *)NULL) {
        // If we just introduced a FogAttrib here, call adjust_to_camera()
        // now.  This maybe isn't the perfect time to call it, but it's
        // good enough; and at this time we have all the information we
        // need for it.
        fog->get_fog()->adjust_to_camera(get_camera_transform());
      }

      if (fancy_bits & PandaNode::FB_cull_callback) {
        PandaNode *node = data.node();
        if (!node->cull_callback(this, data)) {
          return;
        }
      }
    }

    traverse_below(data);
  }
}
