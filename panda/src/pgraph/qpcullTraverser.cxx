// Filename: qpcullTraverser.cxx
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

#include "qpcullTraverser.h"
#include "transformState.h"
#include "renderState.h"
#include "billboardAttrib.h"
#include "cullHandler.h"
#include "dcast.h"
#include "qpgeomNode.h"
#include "colorAttrib.h"
#include "textureAttrib.h"
#include "config_pgraph.h"

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
qpCullTraverser::
qpCullTraverser() {
  _initial_state = RenderState::make_empty();
  _camera_transform = DCAST(TransformState, TransformState::make_identity());
  _render_transform = DCAST(TransformState, TransformState::make_identity());
  _cull_handler = (CullHandler *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_initial_state
//       Access: Public
//  Description: Sets the initial RenderState at the top of the scene
//               graph we are traversing.  If this is not set, the
//               default is the empty state.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
set_initial_state(const RenderState *initial_state) {
  _initial_state = initial_state;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_camera_transform
//       Access: Public
//  Description: Specifies the position of the camera relative to the
//               starting node, without any compensating
//               coordinate-system transforms that might have been
//               introduced for the purposes of rendering.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
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
void qpCullTraverser::
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
void qpCullTraverser::
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
void qpCullTraverser::
set_guard_band(GeometricBoundingVolume *guard_band) {
  _guard_band = guard_band;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_cull_handler
//       Access: Public
//  Description: Specifies the object that will receive the culled
//               Geoms.  This must be set before calling traverse().
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
set_cull_handler(CullHandler *cull_handler) {
  _cull_handler = cull_handler;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::traverse
//       Access: Public
//  Description: Begins the traversal from the indicated node.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
traverse(PandaNode *root) {
  nassertv(_cull_handler != (CullHandler *)NULL);

  r_traverse(root, _render_transform, TransformState::make_identity(),
             _initial_state, _view_frustum, _guard_band);
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::r_traverse
//       Access: Private
//  Description: The recursive traversal implementation.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
r_traverse(PandaNode *node, 
           const TransformState *render_transform,
           const TransformState *net_transform,
           const RenderState *state, 
           GeometricBoundingVolume *view_frustum,
           GeometricBoundingVolume *guard_band) {
  CPT(RenderState) next_state = state;

  if (view_frustum != (GeometricBoundingVolume *)NULL) {
    // If we have a viewing frustum, check to see if the node's
    // bounding volume falls within it.
    const BoundingVolume &node_volume = node->get_bound();
    nassertv(node_volume.is_of_type(GeometricBoundingVolume::get_class_type()));
    const GeometricBoundingVolume *node_gbv =
      DCAST(GeometricBoundingVolume, &node_volume);

    int result = view_frustum->contains(node_gbv);
    if (result == BoundingVolume::IF_no_intersection) {
      // No intersection at all.  Cull.
      if (!qpfake_view_frustum_cull) {
        return;
      }

      // If we have fake view-frustum culling enabled, instead of
      // actually culling an object we simply force it to be drawn in
      // red wireframe.
      view_frustum = (GeometricBoundingVolume *)NULL;
      CPT(RenderState) fake_effect = RenderState::make
        (ColorAttrib::make_flat(Colorf(1.0f, 0.0f, 0.0f, 1.0f)),
         TextureAttrib::make_off(),
         1000);
      next_state = next_state->compose(fake_effect);

    } else if ((result & BoundingVolume::IF_all) != 0) {
      // The node and its descendants are completely enclosed within
      // the frustum.  No need to cull further.
      view_frustum = (GeometricBoundingVolume *)NULL;

    } else {
      if (node->is_final()) {
        // The bounding volume is partially, but not completely,
        // within the viewing frustum.  Normally we'd keep testing
        // child bounding volumes as we continue down.  But this node
        // has the "final" flag, so the user is claiming that there is
        // some important reason we should consider everything visible
        // at this point.  So be it.
        view_frustum = (GeometricBoundingVolume *)NULL;
      }
    }
  }

  CPT(TransformState) next_render_transform = render_transform;
  CPT(TransformState) next_net_transform = net_transform;
  PT(GeometricBoundingVolume) next_view_frustum = view_frustum;
  PT(GeometricBoundingVolume) next_guard_band = guard_band;

  const TransformState *transform = node->get_transform();
  if (!transform->is_identity()) {
    next_render_transform = render_transform->compose(transform);
    next_net_transform = net_transform->compose(transform);

    if ((view_frustum != (GeometricBoundingVolume *)NULL) ||
        (guard_band != (GeometricBoundingVolume *)NULL)) {
      // We need to move the viewing frustums into the node's
      // coordinate space by applying the node's inverse transform.
      if (transform->is_singular()) {
        // But we can't invert a singular transform!  Instead of
        // trying, we'll just give up on frustum culling from this
        // point down.
        view_frustum = (GeometricBoundingVolume *)NULL;
        guard_band = (GeometricBoundingVolume *)NULL;

      } else {
        CPT(TransformState) inv_transform = 
          transform->invert_compose(TransformState::make_identity());
        
        if (view_frustum != (GeometricBoundingVolume *)NULL) {
          next_view_frustum = DCAST(GeometricBoundingVolume, view_frustum->make_copy());
          next_view_frustum->xform(inv_transform->get_mat());
        }
        
        if (guard_band != (GeometricBoundingVolume *)NULL) {
          next_guard_band = DCAST(GeometricBoundingVolume, guard_band->make_copy());
          next_guard_band->xform(inv_transform->get_mat());
        }
      }
    }
  }

  next_state = next_state->compose(node->get_state());

  const BillboardAttrib *billboard = state->get_billboard();
  if (billboard != (const BillboardAttrib *)NULL) {
    // Got to apply a billboard transform here.
    CPT(TransformState) billboard_transform = 
      billboard->do_billboard(net_transform, _camera_transform);
    next_render_transform = next_render_transform->compose(billboard_transform);
    next_net_transform = next_net_transform->compose(billboard_transform);

    // We can't reliably cull within a billboard, because the geometry
    // might get rotated out of its bounding volume.  So once we get
    // within a billboard, we consider it all visible.
    next_view_frustum = (GeometricBoundingVolume *)NULL;
  }

  if (node->is_geom_node()) {
    qpGeomNode *geom_node;
    DCAST_INTO_V(geom_node, node);
    
    int num_geoms = geom_node->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      Geom *geom = geom_node->get_geom(i);
      CPT(RenderState) geom_state = 
        next_state->compose(geom_node->get_geom_state(i));
      _cull_handler->record_geom(geom, next_render_transform, geom_state);
    }
  }

  // Now visit all the node's children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    r_traverse(cr.get_child(i), next_render_transform, next_net_transform,
               next_state, next_view_frustum, next_guard_band);
  }
}
