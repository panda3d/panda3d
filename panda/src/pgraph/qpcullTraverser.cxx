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
      CPT(RenderState) fake_effect = get_fake_view_frustum_cull_effect();
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

  const TransformState *node_transform = node->get_transform();
  if (!node_transform->is_identity()) {
    next_render_transform = render_transform->compose(node_transform);
    next_net_transform = net_transform->compose(node_transform);

    if ((view_frustum != (GeometricBoundingVolume *)NULL) ||
        (guard_band != (GeometricBoundingVolume *)NULL)) {
      // We need to move the viewing frustums into the node's
      // coordinate space by applying the node's inverse transform.
      if (node_transform->is_singular()) {
        // But we can't invert a singular transform!  Instead of
        // trying, we'll just give up on frustum culling from this
        // point down.
        view_frustum = (GeometricBoundingVolume *)NULL;
        guard_band = (GeometricBoundingVolume *)NULL;

      } else {
        CPT(TransformState) inv_transform = 
          node_transform->invert_compose(TransformState::make_identity());
        
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

  const RenderState *node_state = node->get_state();
  next_state = next_state->compose(node_state);

  const BillboardAttrib *billboard = node_state->get_billboard();
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

  if (node_state->has_decal()) {
    start_decal(node, next_render_transform, next_state);

  } else {
    if (node->is_geom_node()) {
      qpGeomNode *geom_node = DCAST(qpGeomNode, node);
      
      // Get all the Geoms, with no decalling.
      int num_geoms = geom_node->get_num_geoms();
      for (int i = 0; i < num_geoms; i++) {
        CullableObject *object = new CullableObject;
        object->_geom = geom_node->get_geom(i);
        object->_state = next_state->compose(geom_node->get_geom_state(i));
        object->_transform = next_render_transform;
        _cull_handler->record_object(object);
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
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::start_decal
//       Access: Private
//  Description: Collects a base node and all of the decals applied to
//               it.  This involves recursing below the base GeomNode
//               to find all the decal geoms; we don't bother to apply
//               any view-frustum culling at this point, and we don't
//               presently support billboards or LOD's within the
//               decals.  Hard to justify the duplicate code this
//               would require.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
start_decal(PandaNode *node, 
            const TransformState *render_transform,
            const RenderState *state) {
  if (!node->is_geom_node()) {
    pgraph_cat.error()
      << "DecalAttrib applied to " << *node << ", not a GeomNode.\n";
    return;
  }

  // Build a chain of CullableObjects.  The head of the chain will be
  // all of the base Geoms in order, followed by an empty
  // CullableObject node, followed by all of the decal Geoms, in
  // order.

  const TransformState *next_render_transform = render_transform;
  const RenderState *next_state = state;

  // Since the CullableObject is a linked list which gets built in
  // LIFO order, we start with the decals.
  CullableObject *decals = (CullableObject *)NULL;
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = num_children - 1; i >= 0; i--) {
    decals =
      r_get_decals(cr.get_child(i), next_render_transform, next_state, decals);
  }

  // Now create a new, empty CullableObject to separate the decals
  // from the non-decals.
  CullableObject *separator = new CullableObject(decals);

  // And now get the base Geoms, again in reverse order.
  CullableObject *object = separator;
  qpGeomNode *geom_node = DCAST(qpGeomNode, node);
      
  // Get all the Geoms, with no decalling.
  int num_geoms = geom_node->get_num_geoms();
  for (int i = num_geoms - 1; i >= 0; i--) {
    object = new CullableObject(object);
    object->_geom = geom_node->get_geom(i);
    object->_state = next_state->compose(geom_node->get_geom_state(i));
    object->_transform = next_render_transform;
  }

  if (object != separator) {
    // Finally, send the whole list down to the CullHandler for
    // processing.  The first Geom in the node now represents the
    // overall state.
    _cull_handler->record_object(object);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::r_get_decals
//       Access: Private
//  Description: Recursively gets all the decals applied to a
//               particular GeomNode.  These are built into a
//               CullableObject list in LIFO order (so that the
//               traversing the list will extract them in the order
//               they were encountered in the scene graph).
////////////////////////////////////////////////////////////////////
CullableObject *qpCullTraverser::
r_get_decals(PandaNode *node, 
             const TransformState *render_transform,
             const RenderState *state,
             CullableObject *decals) {
  const TransformState *node_transform = node->get_transform();
  const RenderState *node_state = node->get_state();

  CPT(TransformState) next_render_transform = 
    render_transform->compose(node_transform);
  CPT(RenderState) next_state =
    state->compose(node_state);

  // First, visit all of the node's children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = num_children - 1; i >= 0; i--) {
    decals =
      r_get_decals(cr.get_child(i), next_render_transform, next_state, decals);
  }

  // Now, tack on any geoms within the node.
  if (node->is_geom_node()) {
    qpGeomNode *geom_node = DCAST(qpGeomNode, node);

    int num_geoms = geom_node->get_num_geoms();
    for (int i = num_geoms - 1; i >= 0; i--) {
      decals = new CullableObject(decals);
      decals->_geom = geom_node->get_geom(i);
      decals->_state = next_state->compose(geom_node->get_geom_state(i));
      decals->_transform = next_render_transform;
    }
  }

  return decals;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::get_fake_view_frustum_cull_effect
//       Access: Private, Static
//  Description: Returns a RenderState for rendering stuff in red
//               wireframe, strictly for the fake_view_frustum_cull
//               effect.
////////////////////////////////////////////////////////////////////
CPT(RenderState) qpCullTraverser::
get_fake_view_frustum_cull_effect() {
  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) effect = (const RenderState *)NULL;
  if (effect == (const RenderState *)NULL) {
    effect = RenderState::make
      (ColorAttrib::make_flat(Colorf(1.0f, 0.0f, 0.0f, 1.0f)),
       TextureAttrib::make_off(),
       1000);
  }
  return effect;
}

