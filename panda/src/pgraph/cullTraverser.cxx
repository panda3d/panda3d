/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullTraverser.cxx
 * @author drose
 * @date 2002-02-23
 */

#include "config_pgraph.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "transformState.h"
#include "renderState.h"
#include "colorAttrib.h"
#include "renderModeAttrib.h"
#include "cullFaceAttrib.h"
#include "depthOffsetAttrib.h"
#include "cullHandler.h"
#include "dcast.h"
#include "geomNode.h"
#include "config_pgraph.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "boundingHexahedron.h"
#include "portalClipper.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomTriangles.h"
#include "geomLinestrips.h"
#include "geomLines.h"
#include "geomVertexWriter.h"

PStatCollector CullTraverser::_nodes_pcollector("Nodes");
PStatCollector CullTraverser::_geom_nodes_pcollector("Nodes:GeomNodes");
PStatCollector CullTraverser::_geoms_pcollector("Geoms");
PStatCollector CullTraverser::_geoms_occluded_pcollector("Geoms:Occluded");

TypeHandle CullTraverser::_type_handle;

/**
 *
 */
CullTraverser::
CullTraverser() :
  _gsg(nullptr),
  _current_thread(Thread::get_current_thread())
{
  _camera_mask = DrawMask::all_on();
  _has_tag_state_key = false;
  _initial_state = RenderState::make_empty();
  _cull_handler = nullptr;
  _portal_clipper = nullptr;
  _effective_incomplete_render = true;
}

/**
 *
 */
CullTraverser::
CullTraverser(const CullTraverser &copy) :
  _gsg(copy._gsg),
  _current_thread(copy._current_thread),
  _scene_setup(copy._scene_setup),
  _camera_mask(copy._camera_mask),
  _has_tag_state_key(copy._has_tag_state_key),
  _tag_state_key(copy._tag_state_key),
  _initial_state(copy._initial_state),
  _view_frustum(copy._view_frustum),
  _cull_handler(copy._cull_handler),
  _portal_clipper(copy._portal_clipper),
  _effective_incomplete_render(copy._effective_incomplete_render)
{
}

/**
 * Sets the SceneSetup object that indicates the initial camera position, etc.
 * This must be called before traversal begins.
 */
void CullTraverser::
set_scene(SceneSetup *scene_setup, GraphicsStateGuardianBase *gsg,
          bool dr_incomplete_render) {
  _scene_setup = scene_setup;
  _gsg = gsg;

  _initial_state = scene_setup->get_initial_state();

  _current_thread = Thread::get_current_thread();

  const Camera *camera = scene_setup->get_camera_node();
  _tag_state_key = camera->get_tag_state_key();
  _has_tag_state_key = !_tag_state_key.empty();
  _camera_mask = camera->get_camera_mask();

  _effective_incomplete_render = _gsg->get_incomplete_render() && dr_incomplete_render;

  _view_frustum = scene_setup->get_view_frustum();
}

/**
 * Begins the traversal from the indicated node.
 */
void CullTraverser::
traverse(const NodePath &root) {
  nassertv(_cull_handler != nullptr);
  nassertv(_scene_setup != nullptr);

  if (allow_portal_cull) {
    // This _view_frustum is in cull_center space Erik: obsolete?
    // PT(GeometricBoundingVolume) vf = _view_frustum;

    GeometricBoundingVolume *local_frustum = nullptr;
    PT(BoundingVolume) bv = _scene_setup->get_lens()->make_bounds();
    if (bv != nullptr) {
      local_frustum = bv->as_geometric_bounding_volume();
    }

    // This local_frustum is in camera space
    PortalClipper portal_viewer(local_frustum, _scene_setup);
    if (debug_portal_cull) {
      portal_viewer.draw_camera_frustum();
    }

    // Store this pointer in this
    set_portal_clipper(&portal_viewer);

    CullTraverserData data(root, TransformState::make_identity(),
                           _initial_state, _view_frustum,
                           _current_thread);

    traverse(data);

    // Finally add the lines to be drawn
    if (debug_portal_cull) {
      portal_viewer.draw_lines();
    }

    // Render the frustum relative to the cull center.
    NodePath cull_center = _scene_setup->get_cull_center();
    CPT(TransformState) transform = cull_center.get_transform(root);

    CullTraverserData my_data(data, portal_viewer._previous);
    my_data._net_transform = my_data._net_transform->compose(transform);
    traverse(my_data);

  } else {
    CullTraverserData data(root, TransformState::make_identity(),
                           _initial_state, _view_frustum,
                           _current_thread);

    do_traverse(data);
  }
}

/**
 * Traverses from the next node with the given data, which has been
 * constructed with the node but has not yet been converted into the node's
 * space.
 */
void CullTraverser::
traverse(CullTraverserData &data) {
  do_traverse(data);
}

/**
 * Traverses all the children of the indicated node, with the given data,
 * which has been converted into the node's space.
 */
void CullTraverser::
traverse_below(CullTraverserData &data) {
  _nodes_pcollector.add_level(1);
  PandaNodePipelineReader *node_reader = data.node_reader();
  PandaNode *node = data.node();

  if (!data.is_this_node_hidden(_camera_mask)) {
    node->add_for_draw(this, data);

    // Check for a decal effect.
    const RenderEffects *node_effects = node_reader->get_effects();
    if (node_effects->has_decal()) {
      // If we *are* implementing decals with DepthOffsetAttribs, apply it
      // now, so that each child of this node gets offset by a tiny amount.
      data._state = data._state->compose(get_depth_offset_state());
#ifndef NDEBUG
      // This is just a sanity check message.
      if (!node->is_geom_node()) {
        pgraph_cat.error()
          << "DecalEffect applied to " << *node << ", not a GeomNode.\n";
      }
#endif
    }
  }

  // Now visit all the node's children.
  PandaNode::Children children = node_reader->get_children();
  node_reader->release();
  int num_children = children.get_num_children();
  if (!node->has_selective_visibility()) {
    for (int i = 0; i < num_children; ++i) {
      CullTraverserData next_data(data, children.get_child(i));
      do_traverse(next_data);
    }
  } else {
    int i = node->get_first_visible_child();
    while (i < num_children) {
      CullTraverserData next_data(data, children.get_child(i));
      do_traverse(next_data);
      i = node->get_next_visible_child(i);
    }
  }
}

/**
 * Should be called when the traverser has finished traversing its scene, this
 * gives it a chance to do any necessary finalization.
 */
void CullTraverser::
end_traverse() {
  _cull_handler->end_traverse();
}

/**
 * Draws an appropriate visualization of the indicated bounding volume.
 */
void CullTraverser::
draw_bounding_volume(const BoundingVolume *vol,
                     const TransformState *internal_transform) const {
  PT(Geom) bounds_viz = make_bounds_viz(vol);

  if (bounds_viz != nullptr) {
    _geoms_pcollector.add_level(2);
    CullableObject *outer_viz =
      new CullableObject(bounds_viz, get_bounds_outer_viz_state(),
                         internal_transform);
    _cull_handler->record_object(outer_viz, this);

    CullableObject *inner_viz =
      new CullableObject(std::move(bounds_viz), get_bounds_inner_viz_state(),
                         internal_transform);
    _cull_handler->record_object(inner_viz, this);
  }
}

/**
 * Returns true if the current node is fully or partially within the viewing
 * area and should be drawn, or false if it (and all of its children) should
 * be pruned.
 */
bool CullTraverser::
is_in_view(CullTraverserData &data) {
  return data.is_in_view(_camera_mask);
}

/**
 * Draws an appropriate visualization of the node's external bounding volume.
 */
void CullTraverser::
show_bounds(CullTraverserData &data, bool tight) {
  PandaNode *node = data.node();
  CPT(TransformState) internal_transform = data.get_internal_transform(this);

  if (tight) {
    PT(Geom) bounds_viz = make_tight_bounds_viz(node);

    if (bounds_viz != nullptr) {
      _geoms_pcollector.add_level(1);
      CullableObject *outer_viz =
        new CullableObject(std::move(bounds_viz), get_bounds_outer_viz_state(),
                           internal_transform);
      _cull_handler->record_object(outer_viz, this);
    }

  } else {
    draw_bounding_volume(node->get_bounds(), internal_transform);

    if (node->is_geom_node()) {
      // Also show the bounding volumes of included Geoms.
      internal_transform = internal_transform->compose(node->get_transform());
      GeomNode *gnode = (GeomNode *)node;
      int num_geoms = gnode->get_num_geoms();
      for (int i = 0; i < num_geoms; ++i) {
        draw_bounding_volume(gnode->get_geom(i)->get_bounds(),
                             internal_transform);
      }
    }
  }
}

/**
 * Returns an appropriate visualization of the indicated bounding volume.
 */
PT(Geom) CullTraverser::
make_bounds_viz(const BoundingVolume *vol) {
  PT(Geom) geom;
  if (vol->is_infinite() || vol->is_empty()) {
    // No way to draw an infinite or empty bounding volume.

  } else if (vol->is_of_type(BoundingSphere::get_class_type())) {
    const BoundingSphere *sphere = DCAST(BoundingSphere, vol);

    static const int num_slices = 16;
    static const int num_stacks = 8;

    PT(GeomVertexData) vdata = new GeomVertexData
      ("bounds", GeomVertexFormat::get_v3(),
       Geom::UH_stream);
    GeomVertexWriter vertex(vdata, InternalName::get_vertex());

    PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_stream);
    for (int sl = 0; sl < num_slices; ++sl) {
      PN_stdfloat longitude0 = (PN_stdfloat)sl / (PN_stdfloat)num_slices;
      PN_stdfloat longitude1 = (PN_stdfloat)(sl + 1) / (PN_stdfloat)num_slices;
      vertex.add_data3(compute_point(sphere, 0.0, longitude0));
      for (int st = 1; st < num_stacks; ++st) {
        PN_stdfloat latitude = (PN_stdfloat)st / (PN_stdfloat)num_stacks;
        vertex.add_data3(compute_point(sphere, latitude, longitude0));
        vertex.add_data3(compute_point(sphere, latitude, longitude1));
      }
      vertex.add_data3(compute_point(sphere, 1.0, longitude0));

      strip->add_next_vertices(num_stacks * 2);
      strip->close_primitive();
    }

    geom = new Geom(vdata);
    geom->add_primitive(strip);

  } else if (vol->is_of_type(BoundingHexahedron::get_class_type())) {
    const BoundingHexahedron *fvol = DCAST(BoundingHexahedron, vol);

    PT(GeomVertexData) vdata = new GeomVertexData
      ("bounds", GeomVertexFormat::get_v3(), Geom::UH_stream);
    vdata->unclean_set_num_rows(8);

    {
      GeomVertexWriter vertex(vdata, InternalName::get_vertex());
      for (int i = 0; i < 8; ++i) {
        vertex.set_data3(fvol->get_point(i));
      }
    }

    PT(GeomLines) lines = new GeomLines(Geom::UH_stream);
    lines->add_vertices(0, 1);
    lines->add_vertices(1, 2);
    lines->add_vertices(2, 3);
    lines->add_vertices(3, 0);

    lines->add_vertices(4, 5);
    lines->add_vertices(5, 6);
    lines->add_vertices(6, 7);
    lines->add_vertices(7, 4);

    lines->add_vertices(0, 4);
    lines->add_vertices(1, 5);
    lines->add_vertices(2, 6);
    lines->add_vertices(3, 7);

    geom = new Geom(vdata);
    geom->add_primitive(lines);

  } else if (vol->is_of_type(FiniteBoundingVolume::get_class_type())) {
    const FiniteBoundingVolume *fvol = DCAST(FiniteBoundingVolume, vol);

    BoundingBox box(fvol->get_min(), fvol->get_max());
    box.local_object();

    PT(GeomVertexData) vdata = new GeomVertexData
      ("bounds", GeomVertexFormat::get_v3(), Geom::UH_stream);
    vdata->unclean_set_num_rows(8);

    {
      GeomVertexWriter vertex(vdata, InternalName::get_vertex());
      for (int i = 0; i < 8; ++i) {
        vertex.set_data3(box.get_point(i));
      }
    }

    PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_stream);
    tris->add_vertices(0, 4, 5);
    tris->add_vertices(0, 5, 1);
    tris->add_vertices(4, 6, 7);
    tris->add_vertices(4, 7, 5);
    tris->add_vertices(6, 2, 3);
    tris->add_vertices(6, 3, 7);
    tris->add_vertices(2, 0, 1);
    tris->add_vertices(2, 1, 3);
    tris->add_vertices(1, 5, 7);
    tris->add_vertices(1, 7, 3);
    tris->add_vertices(2, 6, 4);
    tris->add_vertices(2, 4, 0);

    geom = new Geom(vdata);
    geom->add_primitive(tris);

  } else {
    pgraph_cat.warning()
      << "Don't know how to draw a representation of "
      << vol->get_class_type() << "\n";
  }

  return geom;
}

/**
 * Returns a bounding-box visualization of the indicated node's "tight"
 * bounding volume.
 */
PT(Geom) CullTraverser::
make_tight_bounds_viz(PandaNode *node) const {
  PT(Geom) geom;

  NodePath np = NodePath::any_path(node);

  LPoint3 n, x;
  bool found_any = false;
  node->calc_tight_bounds(n, x, found_any, TransformState::make_identity(),
                          _current_thread);
  if (found_any) {
    PT(GeomVertexData) vdata = new GeomVertexData
      ("bounds", GeomVertexFormat::get_v3(), Geom::UH_stream);
    vdata->unclean_set_num_rows(8);

    {
      GeomVertexWriter vertex(vdata, InternalName::get_vertex(),
                              _current_thread);
      vertex.set_data3(n[0], n[1], n[2]);
      vertex.set_data3(n[0], n[1], x[2]);
      vertex.set_data3(n[0], x[1], n[2]);
      vertex.set_data3(n[0], x[1], x[2]);
      vertex.set_data3(x[0], n[1], n[2]);
      vertex.set_data3(x[0], n[1], x[2]);
      vertex.set_data3(x[0], x[1], n[2]);
      vertex.set_data3(x[0], x[1], x[2]);
    }

    PT(GeomLinestrips) strip = new GeomLinestrips(Geom::UH_stream);

    // We wind one long linestrip around the wireframe cube.  This does
    // require backtracking a few times here and there.
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

    geom = new Geom(vdata);
    geom->add_primitive(strip);
  }

  return geom;
}

/**
 * Returns a point on the surface of the sphere.  latitude and longitude range
 * from 0.0 to 1.0.
 */
LVertex CullTraverser::
compute_point(const BoundingSphere *sphere,
              PN_stdfloat latitude, PN_stdfloat longitude) {
  PN_stdfloat s1, c1;
  csincos(latitude * MathNumbers::pi, &s1, &c1);

  PN_stdfloat s2, c2;
  csincos(longitude * 2.0 * MathNumbers::pi, &s2, &c2);

  LVertex p(s1 * c2, s1 * s2, c1);
  return p * sphere->get_radius() + sphere->get_center();
}

/**
 * Returns a RenderState for rendering the outside surfaces of the bounding
 * volume visualizations.
 */
CPT(RenderState) CullTraverser::
get_bounds_outer_viz_state() {
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    state = RenderState::make
      (ColorAttrib::make_flat(LColor(0.3, 1.0f, 0.5f, 1.0f)),
       RenderModeAttrib::make(RenderModeAttrib::M_wireframe),
       CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise));
  }
  return state;
}

/**
 * Returns a RenderState for rendering the inside surfaces of the bounding
 * volume visualizations.
 */
CPT(RenderState) CullTraverser::
get_bounds_inner_viz_state() {
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    state = RenderState::make
      (ColorAttrib::make_flat(LColor(0.15f, 0.5f, 0.25f, 1.0f)),
       RenderModeAttrib::make(RenderModeAttrib::M_wireframe),
       CullFaceAttrib::make(CullFaceAttrib::M_cull_counter_clockwise));
  }
  return state;
}

/**
 * Returns a RenderState for increasing the DepthOffset by one.
 */
CPT(RenderState) CullTraverser::
get_depth_offset_state() {
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    state = RenderState::make
      (DepthOffsetAttrib::make(1));
  }
  return state;
}
