// Filename: cullTraverser.cxx
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

#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "transformState.h"
#include "renderState.h"
#include "fogAttrib.h"
#include "cullHandler.h"
#include "dcast.h"
#include "geomNode.h"
#include "config_pgraph.h"
#include "boundingSphere.h"
#include "geomSphere.h"
#include "colorAttrib.h"
#include "renderModeAttrib.h"
#include "cullFaceAttrib.h"
#include "depthOffsetAttrib.h"

#ifndef CPPPARSER
PStatCollector CullTraverser::_nodes_pcollector("Nodes");
PStatCollector CullTraverser::_geom_nodes_pcollector("Nodes:GeomNodes");
#endif  // CPPPARSER

TypeHandle CullTraverser::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullTraverser::
CullTraverser() {
  _camera_mask = DrawMask::all_on();
  _initial_state = RenderState::make_empty();
  _depth_offset_decals = false;
  _cull_handler = (CullHandler *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullTraverser::
CullTraverser(const CullTraverser &copy) :
  _scene_setup(copy._scene_setup),
  _camera_mask(copy._camera_mask),
  _initial_state(copy._initial_state),
  _depth_offset_decals(copy._depth_offset_decals),
  _view_frustum(copy._view_frustum),
  _guard_band(copy._guard_band),
  _cull_handler(copy._cull_handler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::traverse
//       Access: Public
//  Description: Begins the traversal from the indicated node.
////////////////////////////////////////////////////////////////////
void CullTraverser::
traverse(const NodePath &root) {
  nassertv(_cull_handler != (CullHandler *)NULL);
  nassertv(_scene_setup != (SceneSetup *)NULL);

  CullTraverserData data(root, get_render_transform(),
                         TransformState::make_identity(),
                         _initial_state, _view_frustum, _guard_band);
  traverse(data);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::traverse
//       Access: Public
//  Description: Traverses from the next node with the given
//               data, which has been constructed with the node but
//               has not yet been converted into the node's space.
////////////////////////////////////////////////////////////////////
void CullTraverser::
traverse(CullTraverserData &data) {
  // Most nodes will have no transform or state, and will not
  // contain decals or require a special cull callback.  As an
  // optimization, we should tag nodes with these properties as
  // being "fancy", and skip this processing for non-fancy nodes.
  if (data.is_in_view(_camera_mask)) {
    PandaNode *node = data.node();

    const RenderEffects *node_effects = node->get_effects();
    if (node_effects->has_show_bounds()) {
      // If we should show the bounding volume for this node, make it
      // up now.
      show_bounds(data);
    }

    data.apply_transform_and_state(this);

    const RenderState *node_state = node->get_state();
    const FogAttrib *fog = node_state->get_fog();
    if (fog != (const FogAttrib *)NULL && fog->get_fog() != (Fog *)NULL) {
      // If we just introduced a FogAttrib here, call adjust_to_camera()
      // now.  This maybe isn't the perfect time to call it, but it's
      // good enough; and at this time we have all the information we
      // need for it.
      fog->get_fog()->adjust_to_camera(get_camera_transform());
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
//     Function: CullTraverser::traverse_below
//       Access: Public
//  Description: Traverses all the children of the indicated node,
//               with the given data, which been converted into the
//               node's space.
////////////////////////////////////////////////////////////////////
void CullTraverser::
traverse_below(CullTraverserData &data) {
  _nodes_pcollector.add_level(1);
  PandaNode *node = data.node();
  const RenderEffects *node_effects = node->get_effects();
  bool has_decal = node_effects->has_decal();
  if (has_decal && !_depth_offset_decals) {
    // Start the three-pass decal rendering if we're not using
    // DepthOffsetAttribs to implement decals.
    start_decal(data);
    
  } else {
    if (node->is_geom_node()) {
      _geom_nodes_pcollector.add_level(1);
      GeomNode *geom_node = DCAST(GeomNode, node);
      
      // Get all the Geoms, with no decalling.
      int num_geoms = geom_node->get_num_geoms();
      for (int i = 0; i < num_geoms; i++) {
        CullableObject *object = new CullableObject(data, geom_node, i);
        _cull_handler->record_object(object);
      }
    }

    if (has_decal) {
      // If we *are* implementing decals with DepthOffsetAttribs,
      // apply it now, so that each child of this node gets offset by
      // a tiny amount.
      data._state = data._state->compose(get_depth_offset_state());
#ifndef NDEBUG
      // This is just a sanity check message.
      if (!node->is_geom_node()) {
        pgraph_cat.error()
          << "DecalEffect applied to " << *node << ", not a GeomNode.\n";
      }
#endif
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
//     Function: CullTraverser::show_bounds
//       Access: Private
//  Description: Draws an appropriate visualization of the node's
//               external bounding volume.
////////////////////////////////////////////////////////////////////
void CullTraverser::
show_bounds(CullTraverserData &data) {
  PandaNode *node = data.node();

  PT(Geom) bounds_viz = make_bounds_viz(node->get_bound());
  if (bounds_viz != (Geom *)NULL) {
    CullableObject *outer_viz = 
      new CullableObject(bounds_viz, get_bounds_outer_viz_state(), 
                         data._render_transform);
    _cull_handler->record_object(outer_viz);

    CullableObject *inner_viz = 
      new CullableObject(bounds_viz, get_bounds_inner_viz_state(), 
                         data._render_transform);
    _cull_handler->record_object(inner_viz);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::make_bounds_viz
//       Access: Private
//  Description: Returns an appropriate visualization of the indicated
//               bounding volume.
////////////////////////////////////////////////////////////////////
PT(Geom) CullTraverser::
make_bounds_viz(const BoundingVolume &vol) {
  PT(Geom) geom;
  if (vol.is_infinite()) {
    // No way to draw an infinite bounding volume.

  } else if (vol.is_of_type(BoundingSphere::get_class_type())) {
    const BoundingSphere *sphere = DCAST(BoundingSphere, &vol);
    
    geom = new GeomSphere;
    PTA_Vertexf verts;
    LPoint3f center = sphere->get_center();
    verts.push_back(center);
    center[0] += sphere->get_radius();
    verts.push_back(center);
    geom->set_coords(verts);
    geom->set_num_prims(1);
    
  } else {
    pgraph_cat.warning()
      << "Don't know how to draw a representation of "
      << vol.get_class_type() << "\n";
  }

  return geom;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_bounds_outer_viz_state
//       Access: Private, Static
//  Description: Returns a RenderState for rendering the outside
//               surfaces of the bounding volume visualizations.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CullTraverser::
get_bounds_outer_viz_state() {
  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) state = (const RenderState *)NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make
      (ColorAttrib::make_flat(Colorf(0.3f, 1.0f, 0.5f, 1.0f)),
       RenderModeAttrib::make(RenderModeAttrib::M_wireframe),
       CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise));
  }
  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_bounds_inner_viz_state
//       Access: Private, Static
//  Description: Returns a RenderState for rendering the inside
//               surfaces of the bounding volume visualizations.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CullTraverser::
get_bounds_inner_viz_state() {
  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) state = (const RenderState *)NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make
      (ColorAttrib::make_flat(Colorf(0.15f, 0.5f, 0.25f, 1.0f)),
       RenderModeAttrib::make(RenderModeAttrib::M_wireframe),
       CullFaceAttrib::make(CullFaceAttrib::M_cull_counter_clockwise));
  }
  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::get_depth_offset_state
//       Access: Private, Static
//  Description: Returns a RenderState for increasing the DepthOffset
//               by one.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CullTraverser::
get_depth_offset_state() {
  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) state = (const RenderState *)NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make
      (DepthOffsetAttrib::make(1));
  }
  return state;
}


////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::start_decal
//       Access: Private
//  Description: Collects a base node and all of the decals applied to
//               it.  This involves recursing below the base GeomNode
//               to find all the decal geoms.
////////////////////////////////////////////////////////////////////
void CullTraverser::
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
  GeomNode *geom_node = DCAST(GeomNode, node);
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
//     Function: CullTraverser::r_get_decals
//       Access: Private
//  Description: Recursively gets all the decals applied to a
//               particular GeomNode.  These are built into a
//               CullableObject list in LIFO order (so that the
//               traversing the list will extract them in the order
//               they were encountered in the scene graph).
////////////////////////////////////////////////////////////////////
CullableObject *CullTraverser::
r_get_decals(CullTraverserData &data, CullableObject *decals) {
  if (data.is_in_view(_camera_mask)) {
    PandaNode *node = data.node();

    const RenderEffects *node_effects = node->get_effects();
    if (node_effects->has_show_bounds()) {
      // If we should show the bounding volume for this node, make it
      // up now.
      show_bounds(data);
    }

    data.apply_transform_and_state(this);

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
      GeomNode *geom_node = DCAST(GeomNode, node);
      
      int num_geoms = geom_node->get_num_geoms();
      for (int i = num_geoms - 1; i >= 0; i--) {
        decals = new CullableObject(data, geom_node, i, decals);
      }
    }
  }

  return decals;
}
