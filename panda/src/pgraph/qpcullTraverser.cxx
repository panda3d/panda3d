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
#include "cullTraverserData.h"
#include "transformState.h"
#include "renderState.h"
#include "billboardAttrib.h"
#include "cullHandler.h"
#include "dcast.h"
#include "qpgeomNode.h"
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

  CullTraverserData data(_render_transform, TransformState::make_identity(),
                         _initial_state, _view_frustum, _guard_band,
                         _camera_transform);
  r_traverse(root, data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::r_traverse
//       Access: Private
//  Description: The recursive traversal implementation.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
r_traverse(PandaNode *node, const CullTraverserData &data) {
  CullTraverserData next_data(data);

  // Most nodes will have no transform or state, and will not
  // contain decals or require a special cull callback.  As an
  // optimization, we should tag nodes with these properties as
  // being "fancy", and skip this processing for non-fancy nodes.

  if (node->get_transform()->is_invalid()) {
    // If the transform is invalid, forget it.
    return;
  }

  if (next_data.is_in_view(node)) {
    next_data.apply_transform_and_state(node);

    if (node->has_cull_callback()) {
      if (!node->cull_callback(next_data)) {
        return;
      }
    }

    const RenderState *node_state = node->get_state();
    if (node_state->has_decal()) {
      start_decal(node, next_data);
      
    } else {
      if (node->is_geom_node()) {
        qpGeomNode *geom_node = DCAST(qpGeomNode, node);
        
        // Get all the Geoms, with no decalling.
        int num_geoms = geom_node->get_num_geoms();
        for (int i = 0; i < num_geoms; i++) {
          CullableObject *object = new CullableObject(next_data, geom_node, i);
          _cull_handler->record_object(object);
        }
      }

      // Now visit all the node's children.
      PandaNode::Children cr = node->get_children();
      int num_children = cr.get_num_children();
      if (node->has_selective_visibility()) {
        int i = node->get_first_visible_child();
        while (i < num_children) {
          r_traverse(cr.get_child(i), next_data);
          i = node->get_next_visible_child(i);
        }

      } else {
        for (int i = 0; i < num_children; i++) {
          r_traverse(cr.get_child(i), next_data);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::start_decal
//       Access: Private
//  Description: Collects a base node and all of the decals applied to
//               it.  This involves recursing below the base GeomNode
//               to find all the decal geoms.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
start_decal(PandaNode *node, const CullTraverserData &data) {
  if (!node->is_geom_node()) {
    pgraph_cat.error()
      << "DecalAttrib applied to " << *node << ", not a GeomNode.\n";
    return;
  }

  // Build a chain of CullableObjects.  The head of the chain will be
  // all of the base Geoms in order, followed by an empty
  // CullableObject node, followed by all of the decal Geoms, in
  // order.

  // Since the CullableObject is a linked list which gets built in
  // LIFO order, we start with the decals.
  CullableObject *decals = (CullableObject *)NULL;
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  if (node->has_selective_visibility()) {
    int i = node->get_first_visible_child();
    while (i < num_children) {
      decals = r_get_decals(cr.get_child(i), data, decals);
      i = node->get_next_visible_child(i);
    }
    
  } else {
    for (int i = num_children - 1; i >= 0; i--) {
      decals = r_get_decals(cr.get_child(i), data, decals);
    }
  }

  // Now create a new, empty CullableObject to separate the decals
  // from the non-decals.
  CullableObject *separator = new CullableObject(decals);

  // And now get the base Geoms, again in reverse order.
  CullableObject *object = separator;
  qpGeomNode *geom_node = DCAST(qpGeomNode, node);
  int num_geoms = geom_node->get_num_geoms();
  for (int i = num_geoms - 1; i >= 0; i--) {
    object = new CullableObject(data, geom_node, i, object);
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
r_get_decals(PandaNode *node, const CullTraverserData &data,
             CullableObject *decals) {
  CullTraverserData next_data(data);

  if (node->get_transform()->is_invalid()) {
    // If the transform is invalid, forget it.
    return decals;
  }

  if (next_data.is_in_view(node)) {
    next_data.apply_transform_and_state(node);

    // First, visit all of the node's children.
    PandaNode::Children cr = node->get_children();
    int num_children = cr.get_num_children();
    if (node->has_selective_visibility()) {
      int i = node->get_first_visible_child();
      while (i < num_children) {
        decals = r_get_decals(cr.get_child(i), next_data, decals);
        i = node->get_next_visible_child(i);
      }
      
    } else {
      for (int i = num_children - 1; i >= 0; i--) {
        decals = r_get_decals(cr.get_child(i), next_data, decals);
      }
    }

    // Now, tack on any geoms within the node.
    if (node->is_geom_node()) {
      qpGeomNode *geom_node = DCAST(qpGeomNode, node);
      
      int num_geoms = geom_node->get_num_geoms();
      for (int i = num_geoms - 1; i >= 0; i--) {
        decals = new CullableObject(next_data, geom_node, i, decals);
      }
    }
  }

  return decals;
}
