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
#include "fogAttrib.h"
#include "cullHandler.h"
#include "dcast.h"
#include "qpgeomNode.h"
#include "config_pgraph.h"


TypeHandle qpCullTraverser::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
qpCullTraverser::
qpCullTraverser() {
  _initial_state = RenderState::make_empty();
  _camera_mask = DrawMask::all_on();
  _camera_transform = DCAST(TransformState, TransformState::make_identity());
  _render_transform = DCAST(TransformState, TransformState::make_identity());
  _cull_handler = (CullHandler *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
qpCullTraverser::
qpCullTraverser(const qpCullTraverser &copy) :
  _initial_state(copy._initial_state),
  _camera_mask(copy._camera_mask),
  _camera_transform(copy._camera_transform),
  _render_transform(copy._render_transform),
  _view_frustum(copy._view_frustum),
  _guard_band(copy._guard_band),
  _cull_handler(copy._cull_handler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::traverse
//       Access: Public
//  Description: Begins the traversal from the indicated node.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
traverse(qpNodePath &root) {
  nassertv(_cull_handler != (CullHandler *)NULL);

  CullTraverserData data(root,
                         _render_transform, TransformState::make_identity(),
                         _initial_state, _view_frustum, _guard_band);
  traverse(data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::traverse
//       Access: Public
//  Description: Traverses from the next node with the given
//               data, which has been constructed with the node but
//               has not yet been converted into the node's space.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
traverse(CullTraverserData &data) {
  // Most nodes will have no transform or state, and will not
  // contain decals or require a special cull callback.  As an
  // optimization, we should tag nodes with these properties as
  // being "fancy", and skip this processing for non-fancy nodes.
  if (data.is_in_view(_camera_mask)) {
    data.apply_transform_and_state(this);

    PandaNode *node = data.node();

    const FogAttrib *fog = node->get_state()->get_fog();
    if (fog != (const FogAttrib *)NULL && fog->get_fog() != (qpFog *)NULL) {
      // If we just introduced a FogAttrib here, call adjust_to_camera()
      // now.  This maybe isn't the perfect time to call it, but it's
      // good enough; and at this time we have all the information we
      // need for it.
      fog->get_fog()->adjust_to_camera(_camera_transform);
    }

    if (node->has_cull_callback()) {
      if (!node->cull_callback(this, data)) {
        return;
      }
    }

    traverse_below(data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::traverse_below
//       Access: Public
//  Description: Traverses all the children of the indicated node,
//               with the given data, which been converted into the
//               node's space.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
traverse_below(const CullTraverserData &data) {
  PandaNode *node = data.node();
  const RenderEffects *node_effects = node->get_effects();
  if (node_effects->has_decal()) {
    start_decal(data);
    
  } else {
    if (node->is_geom_node()) {
      qpGeomNode *geom_node = DCAST(qpGeomNode, node);
      
      // Get all the Geoms, with no decalling.
      int num_geoms = geom_node->get_num_geoms();
      for (int i = 0; i < num_geoms; i++) {
        CullableObject *object = new CullableObject(data, geom_node, i);
        _cull_handler->record_object(object);
      }
    }
    
    // Now visit all the node's children.  We cannot use the
    // node->get_children() interface, because that will keep a read
    // pointer open, and we might end up changing this node during the
    // traversal.
    int num_children = node->get_num_children();
    if (node->has_selective_visibility()) {
      int i = node->get_first_visible_child();
      while (i < num_children) {
        CullTraverserData next_data(data, node->get_child(i));
        traverse(next_data);
        i = node->get_next_visible_child(i);
      }
      
    } else {
      for (int i = 0; i < num_children; i++) {
        CullTraverserData next_data(data, node->get_child(i));
        traverse(next_data);
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
start_decal(const CullTraverserData &data) {
  PandaNode *node = data.node();
  if (!node->is_geom_node()) {
    pgraph_cat.error()
      << "DecalEffect applied to " << *node << ", not a GeomNode.\n";
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
      CullTraverserData next_data(data, cr.get_child(i));
      decals = r_get_decals(next_data, decals);
      i = node->get_next_visible_child(i);
    }
    
  } else {
    for (int i = num_children - 1; i >= 0; i--) {
      CullTraverserData next_data(data, cr.get_child(i));
      decals = r_get_decals(next_data, decals);
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
r_get_decals(CullTraverserData &data, CullableObject *decals) {
  if (data.is_in_view(_camera_mask)) {
    data.apply_transform_and_state(this);

    PandaNode *node = data.node();

    // First, visit all of the node's children.
    int num_children = node->get_num_children();
    if (node->has_selective_visibility()) {
      int i = node->get_first_visible_child();
      while (i < num_children) {
        CullTraverserData next_data(data, node->get_child(i));
        decals = r_get_decals(next_data, decals);
        i = node->get_next_visible_child(i);
      }
      
    } else {
      for (int i = num_children - 1; i >= 0; i--) {
        CullTraverserData next_data(data, node->get_child(i));
        decals = r_get_decals(next_data, decals);
      }
    }

    // Now, tack on any geoms within the node.
    if (node->is_geom_node()) {
      qpGeomNode *geom_node = DCAST(qpGeomNode, node);
      
      int num_geoms = geom_node->get_num_geoms();
      for (int i = num_geoms - 1; i >= 0; i--) {
        decals = new CullableObject(data, geom_node, i, decals);
      }
    }
  }

  return decals;
}
