// Filename: cullTraverser.cxx
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

#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "transformState.h"
#include "renderState.h"
#include "fogAttrib.h"
#include "colorAttrib.h"
#include "renderModeAttrib.h"
#include "cullFaceAttrib.h"
#include "depthOffsetAttrib.h"
#include "cullHandler.h"
#include "dcast.h"
#include "geomNode.h"
#include "config_pgraph.h"
#include "boundingSphere.h"
#include "boundingHexahedron.h"
#include "portalClipper.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomLinestrips.h"
#include "geomVertexWriter.h"

PStatCollector CullTraverser::_nodes_pcollector("Nodes");
PStatCollector CullTraverser::_geom_nodes_pcollector("Nodes:GeomNodes");
PStatCollector CullTraverser::_geoms_pcollector("Geoms");

TypeHandle CullTraverser::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullTraverser::
CullTraverser(GraphicsStateGuardianBase *gsg) :
  _gsg(gsg)
{
  _camera_mask = DrawMask::all_on();
  _has_tag_state_key = false;
  _initial_state = RenderState::make_empty();
  _depth_offset_decals = false;
  _cull_handler = (CullHandler *)NULL;
  _portal_clipper = (PortalClipper *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullTraverser::
CullTraverser(const CullTraverser &copy) :
  _gsg(copy._gsg),
  _scene_setup(copy._scene_setup),
  _camera_mask(copy._camera_mask),
  _has_tag_state_key(copy._has_tag_state_key),
  _tag_state_key(copy._tag_state_key),
  _initial_state(copy._initial_state),
  _depth_offset_decals(copy._depth_offset_decals),
  _view_frustum(copy._view_frustum),
  _guard_band(copy._guard_band),
  _cull_handler(copy._cull_handler),
  _portal_clipper(copy._portal_clipper)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::traverse
//       Access: Public
//  Description: Begins the traversal from the indicated node.
////////////////////////////////////////////////////////////////////
void CullTraverser::
traverse(const NodePath &root, bool python_cull_control) {
  nassertv(_cull_handler != (CullHandler *)NULL);
  nassertv(_scene_setup != (SceneSetup *)NULL);

  if (allow_portal_cull || python_cull_control) {
    // This _view_frustum is in cull_center space
    PT(GeometricBoundingVolume) vf = _view_frustum;

    GeometricBoundingVolume *local_frustum = NULL;
    PT(BoundingVolume) bv = _scene_setup->get_lens()->make_bounds();
    if (bv != (BoundingVolume *)NULL &&
        bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
      
      local_frustum = DCAST(GeometricBoundingVolume, bv);
    }
      
    // This local_frustum is in camera space
    PortalClipper portal_viewer(local_frustum, _scene_setup);
    portal_viewer.draw_camera_frustum();
    
    // store this pointer in this
    set_portal_clipper(&portal_viewer);

    CullTraverserData data(root, get_world_transform(),
                           _initial_state, _view_frustum, 
                           _guard_band);
    
    traverse(data);
    
    // finally add the lines to be drawn
    portal_viewer.draw_lines();
    
    // Render the frustum relative to the cull center.
    NodePath cull_center = _scene_setup->get_cull_center();
    CPT(TransformState) transform = cull_center.get_transform(root);
    
    CullTraverserData my_data(data, portal_viewer._previous);
    my_data._modelview_transform = my_data._modelview_transform->compose(transform);
    traverse(my_data);

  } else {
    CullTraverserData data(root, get_world_transform(),
                           _initial_state, _view_frustum, 
                           _guard_band);
    
    traverse(data);
  }
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
    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam() << "\n" << data._node_path << "\n";
    }

    PandaNode *node = data.node();

    const RenderEffects *node_effects = node->get_effects();
    if (node_effects->has_show_bounds()) {
      // If we should show the bounding volume for this node, make it
      // up now.
      show_bounds(data, node_effects->has_show_tight_bounds());
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
      _geoms_pcollector.add_level(num_geoms);
      for (int i = 0; i < num_geoms; i++) {
        CullableObject *object = new CullableObject(data, geom_node, i);
        _cull_handler->record_object(object, this);
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
show_bounds(CullTraverserData &data, bool tight) {
  PandaNode *node = data.node();

  if (tight) {
    PT(Geom) bounds_viz = make_tight_bounds_viz(node);

    if (bounds_viz != (Geom *)NULL) {
      _geoms_pcollector.add_level(1);
      CullableObject *outer_viz = 
        new CullableObject(bounds_viz, get_bounds_outer_viz_state(), 
                           data._modelview_transform);
      _cull_handler->record_object(outer_viz, this);
    }
    
  } else {
    PT(Geom) bounds_viz = make_bounds_viz(node->get_bound());

    if (bounds_viz != (Geom *)NULL) {
      _geoms_pcollector.add_level(2);
      CullableObject *outer_viz = 
        new CullableObject(bounds_viz, get_bounds_outer_viz_state(), 
                           data._modelview_transform);
      _cull_handler->record_object(outer_viz, this);
      
      CullableObject *inner_viz = 
        new CullableObject(bounds_viz, get_bounds_inner_viz_state(), 
                           data._modelview_transform);
      _cull_handler->record_object(inner_viz, this);
    }
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

    static const int num_slices = 16;
    static const int num_stacks = 8;

    PT(GeomVertexData) vdata = new GeomVertexData
      ("bounds", GeomVertexFormat::get_v3(),
       Geom::UH_stream);
    GeomVertexWriter vertex(vdata, InternalName::get_vertex());
    
    PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_stream);
    for (int sl = 0; sl < num_slices; ++sl) {
      float longitude0 = (float)sl / (float)num_slices;
      float longitude1 = (float)(sl + 1) / (float)num_slices;
      vertex.add_data3f(compute_point(sphere, 0.0, longitude0));
      for (int st = 1; st < num_stacks; ++st) {
        float latitude = (float)st / (float)num_stacks;
        vertex.add_data3f(compute_point(sphere, latitude, longitude0));
        vertex.add_data3f(compute_point(sphere, latitude, longitude1));
      }
      vertex.add_data3f(compute_point(sphere, 1.0, longitude0));
      
      strip->add_next_vertices(num_stacks * 2);
      strip->close_primitive();
    }
    
    geom = new Geom;
    geom->set_vertex_data(vdata);
    geom->add_primitive(strip);

  } else {
    pgraph_cat.warning()
      << "Don't know how to draw a representation of "
      << vol.get_class_type() << "\n";
  }

  return geom;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::make_tight_bounds_viz
//       Access: Private
//  Description: Returns a bounding-box visualization of the indicated
//               node's "tight" bounding volume.
////////////////////////////////////////////////////////////////////
PT(Geom) CullTraverser::
make_tight_bounds_viz(PandaNode *node) {
  PT(Geom) geom;

  NodePath np = NodePath::any_path(node);

  LPoint3f n, x;
  bool found_any = false;
  node->calc_tight_bounds(n, x, found_any, TransformState::make_identity());
  if (found_any) {
    PT(GeomVertexData) vdata = new GeomVertexData
      ("bounds", GeomVertexFormat::get_v3(),
      Geom::UH_stream);
    GeomVertexWriter vertex(vdata, InternalName::get_vertex());
    
    vertex.add_data3f(n[0], n[1], n[2]);
    vertex.add_data3f(n[0], n[1], x[2]);
    vertex.add_data3f(n[0], x[1], n[2]);
    vertex.add_data3f(n[0], x[1], x[2]);
    vertex.add_data3f(x[0], n[1], n[2]);
    vertex.add_data3f(x[0], n[1], x[2]);
    vertex.add_data3f(x[0], x[1], n[2]);
    vertex.add_data3f(x[0], x[1], x[2]);
  
    PT(GeomLinestrips) strip = new GeomLinestrips(Geom::UH_stream);

    // We wind one long linestrip around the wireframe cube.  This
    // does require backtracking a few times here and there.
    strip->add_vertex(0);
    strip->add_vertex(1);
    strip->add_vertex(3);
    strip->add_vertex(2);
    strip->add_vertex(0);
    strip->add_vertex(4);
    strip->add_vertex(5);
    strip->add_vertex(7);
    strip->add_vertex(6);
    strip->add_vertex(4);
    strip->add_vertex(6);
    strip->add_vertex(2);
    strip->add_vertex(3);
    strip->add_vertex(7);
    strip->add_vertex(5);
    strip->add_vertex(1);
    strip->close_primitive();
      
    PT(Geom) geom = new Geom;
    geom->set_vertex_data(vdata);
    geom->add_primitive(strip);
    geom = geom.p();
  }

  return geom;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::compute_point
//       Access: Private, Static
//  Description: Returns a point on the surface of the sphere.
//               latitude and longitude range from 0.0 to 1.0.  
////////////////////////////////////////////////////////////////////
Vertexf CullTraverser::
compute_point(const BoundingSphere *sphere, 
              float latitude, float longitude) {
  float s1, c1;
  csincos(latitude * MathNumbers::pi_f, &s1, &c1);

  float s2, c2;
  csincos(longitude * 2.0f * MathNumbers::pi_f, &s2, &c2);

  Vertexf p(s1 * c2, s1 * s2, c1);
  return p * sphere->get_radius() + sphere->get_center();
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
  _geoms_pcollector.add_level(num_geoms);
  for (int i = num_geoms - 1; i >= 0; i--) {
    object = new CullableObject(data, geom_node, i, object);
  }

  if (object != separator) {
    // Finally, send the whole list down to the CullHandler for
    // processing.  The first Geom in the node now represents the
    // overall state.
    _cull_handler->record_object(object, this);
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
      show_bounds(data, node_effects->has_show_tight_bounds());
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
      _geoms_pcollector.add_level(num_geoms);
      for (int i = num_geoms - 1; i >= 0; i--) {
        decals = new CullableObject(data, geom_node, i, decals);
      }
    }
  }

  return decals;
}
