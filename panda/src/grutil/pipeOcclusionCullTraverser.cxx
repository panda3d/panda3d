/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipeOcclusionCullTraverser.cxx
 * @author drose
 * @date 2007-05-29
 */

#include "pipeOcclusionCullTraverser.h"
#include "graphicsEngine.h"
#include "graphicsPipe.h"
#include "drawCullHandler.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "geomVertexWriter.h"
#include "geomTristrips.h"
#include "geomTriangles.h"
#include "pStatTimer.h"
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "colorWriteAttrib.h"
#include "colorAttrib.h"
#include "cullBinManager.h"
#include "configVariableInt.h"
#include "config_grutil.h"
#include "pnmImage.h"

PStatCollector PipeOcclusionCullTraverser::_setup_occlusion_pcollector("Cull:Occlusion:Setup");
PStatCollector PipeOcclusionCullTraverser::_draw_occlusion_pcollector("Cull:Occlusion:Occluders");
PStatCollector PipeOcclusionCullTraverser::_test_occlusion_pcollector("Cull:Occlusion:Test");
PStatCollector PipeOcclusionCullTraverser::_finish_occlusion_pcollector("Cull:Occlusion:Finish");

PStatCollector PipeOcclusionCullTraverser::_occlusion_untested_pcollector("Occlusion results:Not tested");
PStatCollector PipeOcclusionCullTraverser::_occlusion_passed_pcollector("Occlusion results:Visible");
PStatCollector PipeOcclusionCullTraverser::_occlusion_failed_pcollector("Occlusion results:Occluded");
PStatCollector PipeOcclusionCullTraverser::_occlusion_tests_pcollector("Occlusion tests");

TypeHandle PipeOcclusionCullTraverser::_type_handle;

static ConfigVariableInt min_occlusion_vertices
("min-occlusion-vertices", 300,
 PRC_DESC("The minimum number of vertices a PandaNode or Geom must contain "
          "in order to perform an occlusion query for it.  Nodes and Geoms "
          "smaller than this will be rendered directly, without bothering "
          "with an occlusion query."));

static ConfigVariableInt max_occlusion_vertices
("max-occlusion-vertices", 3000,
 PRC_DESC("The maximum number of vertices that may be included in a PandaNode "
          "and its descendents in order to perform an occlusion query for "
          "it.  Subgraphs whose total vertex count exceeds this number will "
          "be subdivided further before performing an occlusion test--the "
          "hope is that we can eventually get to a finer-grained answer.  "
          "GeomNodes and Geoms will not be subdivided, regardless of this "
          "limit."));

static ConfigVariableBool show_occlusion
("show-occlusion", false,
 PRC_DESC("Set this true to visualize the efforts of the occlusion test."));

static ConfigVariableInt occlusion_size
("occlusion-size", "256 256",
 PRC_DESC("Specify the x y size of the buffer used for occlusion testing."));

static ConfigVariableInt occlusion_depth_bits
("occlusion-depth-bits", 1,
 PRC_DESC("The minimum number of depth bits requested for the occlusion "
          "buffer."));

/**
 *
 */
PipeOcclusionCullTraverser::
PipeOcclusionCullTraverser(GraphicsOutput *host) {
  _live = false;
  GraphicsStateGuardian *gsg = host->get_gsg();

  GraphicsThreadingModel threading_model = gsg->get_threading_model();
  nassertv(threading_model.get_cull_name() == threading_model.get_draw_name());
  if (!gsg->get_supports_occlusion_query()) {
    grutil_cat.info()
      << "Occlusion queries are not supported by graphics pipe.\n";
    return;
  }

  GraphicsEngine *engine = gsg->get_engine();
  GraphicsPipe *pipe = gsg->get_pipe();

  FrameBufferProperties fb_prop;
  fb_prop.set_depth_bits(occlusion_depth_bits);
  WindowProperties win_prop;
  if (occlusion_size.get_num_words() < 2) {
    win_prop.set_size(occlusion_size, occlusion_size);
  } else {
    win_prop.set_size(occlusion_size[0], occlusion_size[1]);
  }

  _buffer = engine->make_output(pipe, "occlusion", 0, fb_prop, win_prop,
                                GraphicsPipe::BF_refuse_window,
                                gsg, host->get_host());
  nassertv(_buffer != nullptr);

  // This buffer isn't really active--we render it by hand; we don't want the
  // GraphicsEngine to render it.
  _buffer->set_active(0);

  _display_region = _buffer->make_display_region();
  _internal_cull_handler = nullptr;

  make_sphere();
  make_box();
  make_solid_test_state();

  _live = true;
}

/**
 *
 */
void PipeOcclusionCullTraverser::
set_scene(SceneSetup *scene_setup, GraphicsStateGuardianBase *gsgbase,
          bool dr_incomplete_render) {
  CullTraverser::set_scene(scene_setup, gsgbase, dr_incomplete_render);
  if (!_live) {
    return;
  }

  PStatTimer timer(_setup_occlusion_pcollector);

  GraphicsStateGuardian *gsg = _buffer->get_gsg();
  nassertv(gsg == gsgbase);

  Thread *current_thread = get_current_thread();
  if (!_buffer->begin_frame(GraphicsOutput::FM_render, current_thread)) {
    grutil_cat.error() << "begin_frame failed\n";
    return;
  }
  _buffer->clear(current_thread);

  DisplayRegionPipelineReader dr_reader(_display_region, current_thread);

  _buffer->change_scenes(&dr_reader);
  gsg->prepare_display_region(&dr_reader);

  _scene = new SceneSetup(*scene_setup);
  _scene->set_display_region(_display_region);
  _scene->set_viewport_size(_display_region->get_pixel_width(),
                            _display_region->get_pixel_height());

  if (_scene->get_cull_center() != _scene->get_camera_path()) {
    // This camera has a special cull center set.  For the purposes of
    // occlusion culling, we want to render the scene from the cull center,
    // not from the camera root.
    NodePath cull_center = _scene->get_cull_center();
    NodePath scene_parent = _scene->get_scene_root().get_parent(current_thread);
    CPT(TransformState) camera_transform = cull_center.get_transform(scene_parent, current_thread);
    CPT(TransformState) world_transform = scene_parent.get_transform(cull_center, current_thread);
    CPT(TransformState) cs_world_transform = _scene->get_cs_transform()->compose(world_transform);
    _scene->set_camera_transform(camera_transform);
    _scene->set_world_transform(world_transform);
    _scene->set_cs_world_transform(cs_world_transform);

    // We use this to recover the original net_transform.
    _inv_cs_world_transform = cs_world_transform->get_inverse();
  } else {
    _inv_cs_world_transform = _scene->get_cs_world_transform()->get_inverse();
  }

  nassertv(_scene->get_cs_transform() == scene_setup->get_cs_transform());

  gsg->set_scene(_scene);
  if (!gsg->begin_scene()) {
    grutil_cat.error() << "begin_scene failed\n";
    return;
  }

  // Hijack the default cull handler so we can perform all of the occlusion
  // tests on a per-object basis, and then query the results at the end of the
  // traversal.
  _true_cull_handler = get_cull_handler();
  set_cull_handler(this);

  _internal_cull_handler = new DrawCullHandler(gsg);
  _internal_trav = new CullTraverser;
  _internal_trav->set_cull_handler(_internal_cull_handler);
  _internal_trav->set_scene(_scene, gsg, dr_incomplete_render);
  _internal_trav->set_view_frustum(get_view_frustum());
  _internal_trav->set_camera_mask(_occlusion_mask);

  _current_query = nullptr;
  _next_query = nullptr;

  // Begin by rendering all the occluders into our internal scene.
  PStatTimer timer2(_draw_occlusion_pcollector);
  _internal_trav->traverse(_scene->get_scene_root());
}

/**
 * Should be called when the traverser has finished traversing its scene, this
 * gives it a chance to do any necessary finalization.
 */
void PipeOcclusionCullTraverser::
end_traverse() {
  if (!_live) {
    return;
  }

  PStatTimer timer(_finish_occlusion_pcollector);
  GraphicsStateGuardian *gsg = _buffer->get_gsg();
  Thread *current_thread = get_current_thread();

  _current_query = nullptr;
  _next_query = nullptr;

  PendingObjects::iterator oi;
  for (oi = _pending_objects.begin(); oi != _pending_objects.end(); ++oi) {
    PendingObject &pobj = (*oi);
    if (pobj._query == nullptr) {
      _occlusion_untested_pcollector.add_level(1);
      _true_cull_handler->record_object(pobj._object, this);
    } else {
      int num_fragments = pobj._query->get_num_fragments();
      if (num_fragments != 0) {
        _occlusion_passed_pcollector.add_level(1);
        _true_cull_handler->record_object(pobj._object, this);
      } else {
        _occlusion_failed_pcollector.add_level(1);
        delete pobj._object;
      }
    }

    // The CullableObject has by now either been recorded (which will
    // eventually delete it) or deleted directly.
#ifndef NDEBUG
    pobj._object = nullptr;
#endif  // NDEBUG
  }
  _pending_objects.clear();
  CullTraverser::end_traverse();

  gsg->end_scene();
  _buffer->end_frame(GraphicsOutput::FM_render, current_thread);

  _buffer->begin_flip();
  _buffer->end_flip();

  delete _internal_cull_handler;
  _internal_cull_handler = nullptr;

  _occlusion_untested_pcollector.flush_level();
  _occlusion_passed_pcollector.flush_level();
  _occlusion_failed_pcollector.flush_level();
  _occlusion_tests_pcollector.flush_level();
}

/**
 * Returns a Texture that can be used to visualize the efforts of the
 * occlusion cull.
 */
Texture *PipeOcclusionCullTraverser::
get_texture() {
  if (_texture != nullptr) {
    return _texture;
  }

  _texture = new Texture("occlusion");

  if (!_live) {
    // If we're not live, just create a default, black texture.
    PNMImage image(1, 1);
    _texture->load(image);

  } else {
    // Otherwise, create a render-to-texture.
    _buffer->add_render_texture(_texture, GraphicsOutput::RTM_bind_or_copy);
  }

  return _texture;
}

/**
 *
 */
bool PipeOcclusionCullTraverser::
is_in_view(CullTraverserData &data) {
  _next_query = nullptr;

  if (!CullTraverser::is_in_view(data)) {
    return false;
  }
  if (!_live) {
    return true;
  }

  if (_current_query != nullptr) {
    // We've already performed an occlusion test for some ancestor of this
    // node; no need to perform another.
    return true;
  }

  PandaNode *node = data.node();
  PandaNodePipelineReader *node_reader = data.node_reader();

  if (node_reader->get_nested_vertices() < min_occlusion_vertices) {
    // Never mind; let this puny one slide.
    return true;
  }

  if (node->is_geom_node() ||
      node_reader->is_final() ||
      node_reader->get_effects()->has_show_bounds() ||
      node_reader->get_nested_vertices() <= max_occlusion_vertices) {
    // In any of these cases, there's sufficient reason to perform an
    // occlusion test on this particular node.  Do it.

    CPT(BoundingVolume) vol = node_reader->get_bounds();
    CPT(TransformState) net_transform = data.get_net_transform(this);
    CPT(TransformState) internal_transform;

    CPT(Geom) geom;
    if (get_volume_viz(vol, geom, net_transform, internal_transform)) {
      _next_query =
        perform_occlusion_test(geom, net_transform, internal_transform);
    }
  }

  return true;
}

/**
 * Traverses all the children of the indicated node, with the given data,
 * which has been converted into the node's space.
 */
void PipeOcclusionCullTraverser::
traverse_below(CullTraverserData &data) {
  // Save and restore _current_query, and clear _next_query, for traversing
  // the children of this node.
  PT(OcclusionQueryContext) prev_query = _current_query;
  if (_next_query != nullptr) {
    _current_query = _next_query;
  }
  _next_query = nullptr;

  CullTraverser::traverse_below(data);

  _current_query = prev_query;
  _next_query = nullptr;
}

/**
 * This callback function is intended to be overridden by a derived class.
 * This is called as each Geom is discovered by the CullTraverser.
 *
 * We do a sneaky trick in making PipeOcclusionCullTraverser inherit from both
 * CullTraverser and CullHandler--the traverser is its own handler!  This is
 * the normal callback into the traverser for rendering objects.  We respond
 * to this by firing off an occlusion test, and queuing up the object until
 * the end of the scene.
 */
void PipeOcclusionCullTraverser::
record_object(CullableObject *object, const CullTraverser *traverser) {
  nassertv(traverser == this);
  PendingObject pobj(object);

  Thread *current_thread = get_current_thread();

  if (_next_query != nullptr) {
    // We have just performed an occlusion query for this node.  Don't perform
    // another one.
    pobj._query = _next_query;

  } else if (_current_query != nullptr) {
    // We have previously performed an occlusion query for this node or some
    // ancestor.  Don't perform another one.
    pobj._query = _current_query;

  } else if (object->_geom->get_nested_vertices(current_thread) < min_occlusion_vertices) {
    // This object is too small to bother testing for occlusions.

  } else {
    // Issue an occlusion test for this object.
    CPT(BoundingVolume) vol = object->_geom->get_bounds(current_thread);
    CPT(TransformState) net_transform = _inv_cs_world_transform->compose(object->_internal_transform);
    CPT(TransformState) internal_transform;
    CPT(Geom) geom;
    if (get_volume_viz(vol, geom, net_transform, internal_transform)) {
      pobj._query =
        perform_occlusion_test(geom, net_transform, internal_transform);
    }
  }

  _pending_objects.push_back(pobj);
}

/**
 * Constructs a unit sphere for testing visibility of bounding spheres.
 */
void PipeOcclusionCullTraverser::
make_sphere() {
  ConfigVariableInt num_slices("num-slices", 16);
  ConfigVariableInt num_stacks("num-stacks", 8);

  // static const int num_slices = 16; static const int num_stacks = 8;

  PT(GeomVertexData) vdata = new GeomVertexData
    ("occlusion_sphere", GeomVertexFormat::get_v3(), Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());

  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_stream);
  for (int sl = 0; sl < num_slices; ++sl) {
    PN_stdfloat longitude0 = (PN_stdfloat)sl / (PN_stdfloat)num_slices;
    PN_stdfloat longitude1 = (PN_stdfloat)(sl + 1) / (PN_stdfloat)num_slices;
    vertex.add_data3(compute_sphere_point(0.0, longitude0));
    for (int st = 1; st < num_stacks; ++st) {
      PN_stdfloat latitude = (PN_stdfloat)st / (PN_stdfloat)num_stacks;
      vertex.add_data3(compute_sphere_point(latitude, longitude0));
      vertex.add_data3(compute_sphere_point(latitude, longitude1));
    }
    vertex.add_data3(compute_sphere_point(1.0, longitude0));

    strip->add_next_vertices(num_stacks * 2);
    strip->close_primitive();
  }

  _sphere_geom = new Geom(vdata);
  _sphere_geom->add_primitive(strip);
}

/**
 * Returns a point on the surface of the unit sphere.  latitude and longitude
 * range from 0.0 to 1.0.
 */
LVertex PipeOcclusionCullTraverser::
compute_sphere_point(PN_stdfloat latitude, PN_stdfloat longitude) {
  PN_stdfloat s1, c1;
  csincos(latitude * MathNumbers::pi, &s1, &c1);

  PN_stdfloat s2, c2;
  csincos(longitude * 2.0f * MathNumbers::pi, &s2, &c2);

  LVertex p(s1 * c2, s1 * s2, c1);
  return p;
}

/**
 * Constructs a unit box for testing visibility of bounding boxes.
 */
void PipeOcclusionCullTraverser::
make_box() {
  PT(GeomVertexData) vdata = new GeomVertexData
    ("occlusion_box", GeomVertexFormat::get_v3(), Geom::UH_static);
  vdata->unclean_set_num_rows(8);

  {
    GeomVertexWriter vertex(vdata, InternalName::get_vertex());
    vertex.set_data3(0.0f, 0.0f, 0.0f);
    vertex.set_data3(0.0f, 0.0f, 1.0f);
    vertex.set_data3(0.0f, 1.0f, 0.0f);
    vertex.set_data3(0.0f, 1.0f, 1.0f);
    vertex.set_data3(1.0f, 0.0f, 0.0f);
    vertex.set_data3(1.0f, 0.0f, 1.0f);
    vertex.set_data3(1.0f, 1.0f, 0.0f);
    vertex.set_data3(1.0f, 1.0f, 1.0f);
  }

  PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);
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

  _box_geom = new Geom(vdata);
  _box_geom->add_primitive(tris);
}

/**
 * Creates the RenderState appropriate to rendering the occlusion test
 * geometry invisibly.
 */
void PipeOcclusionCullTraverser::
make_solid_test_state() {
  _solid_test_state = RenderState::make
    (DepthWriteAttrib::make(DepthWriteAttrib::M_off),
     DepthTestAttrib::make(DepthTestAttrib::M_less),
     ColorWriteAttrib::make(ColorWriteAttrib::C_off));
}

/**
 * Chooses a suitable Geom to render the indicated bounding volume, and fills
 * geom and local_transform with the appropriate values.  Returns true if the
 * bounding volume can be rendered, false if there is no suitable
 * visualization for it.
 *
 * On entry, net_transform should be filled with the net transform to the
 * bounding volume.  On exit (when return value is true), it will be composed
 * with a suitable local transform to render the bounding volume properly, and
 * internal_transform will also be filled with the appropriate transform.
 */
bool PipeOcclusionCullTraverser::
get_volume_viz(const BoundingVolume *vol,
               CPT(Geom) &geom,  // OUT
               CPT(TransformState) &net_transform, // IN-OUT
               CPT(TransformState) &internal_transform  // OUT
               ) {
  if (vol->is_infinite() || vol->is_empty()) {
    return false;
  }

  if (vol->is_exact_type(BoundingSphere::get_class_type())) {
    const BoundingSphere *sphere = DCAST(BoundingSphere, vol);
    CPT(TransformState) local_transform =
      TransformState::make_pos_hpr_scale(sphere->get_center(),
                                         LVecBase3(0, 0, 0),
                                         sphere->get_radius());
    net_transform = net_transform->compose(local_transform);

    CPT(TransformState) modelview_transform =
      _internal_trav->get_world_transform()->compose(net_transform);

    // See if the bounding sphere is clipped by the near plane.  If it is, the
    // occlusion test may fail, so we won't bother performing it for this
    // object.  Anyway, it's not occluded by anything, since it's intersecting
    // the near plane.
    const LPoint3 &center = modelview_transform->get_pos();
    const LVecBase3 &radius = modelview_transform->get_scale();
    if (center[1] - radius[1] < 0.0f) {
      return false;
    }

    // Construct the internal transform for the internal traverser.
    internal_transform = _internal_trav->get_scene()->
      get_cs_transform()->compose(modelview_transform);

    // The sphere looks good.
    geom = _sphere_geom;
    return true;

  } else if (vol->is_exact_type(BoundingBox::get_class_type())) {
    const BoundingBox *box = DCAST(BoundingBox, vol);
    CPT(TransformState) local_transform =
      TransformState::make_pos_hpr_scale(box->get_minq(),
                                         LVecBase3(0, 0, 0),
                                         box->get_maxq() - box->get_minq());
    net_transform = net_transform->compose(local_transform);

    CPT(TransformState) modelview_transform =
      _internal_trav->get_world_transform()->compose(net_transform);

    // See if the bounding box is clipped by the near plane.  If it is, the
    // occlusion test may fail, so we won't bother performing it for this
    // object.  Anyway, it's not occluded by anything, since it's intersecting
    // the near plane.
    static const LPoint3 points[8] = {
      LPoint3(0.0f, 0.0f, 0.0f),
      LPoint3(0.0f, 0.0f, 1.0f),
      LPoint3(0.0f, 1.0f, 0.0f),
      LPoint3(0.0f, 1.0f, 1.0f),
      LPoint3(1.0f, 0.0f, 0.0f),
      LPoint3(1.0f, 0.0f, 1.0f),
      LPoint3(1.0f, 1.0f, 0.0f),
      LPoint3(1.0f, 1.0f, 1.0f),
    };
    const LMatrix4 &mat = modelview_transform->get_mat();
    for (int i = 0; i < 8; ++i) {
      LPoint3 p = points[i] * mat;
      if (p[1] < 0.0f) {
        return false;
      }
    }

    // Construct the internal transform for the internal traverser.
    internal_transform = _internal_trav->get_scene()->
      get_cs_transform()->compose(modelview_transform);

    // The box looks good.
    geom = _box_geom;
    return true;
  }

  // Don't have a suitable representation for this bounding volume.
  return false;
}

/**
 * Renders the indicated geometry in the internal scene to test its
 * visibility.
 */
PT(OcclusionQueryContext) PipeOcclusionCullTraverser::
perform_occlusion_test(const Geom *geom, const TransformState *net_transform,
                       const TransformState *internal_transform) {
  _occlusion_tests_pcollector.add_level(1);
  PStatTimer timer(_test_occlusion_pcollector);

  GraphicsStateGuardian *gsg = _buffer->get_gsg();

  gsg->begin_occlusion_query();

  CullableObject *viz =
    new CullableObject(geom, _solid_test_state, internal_transform);

  static ConfigVariableBool test_occlude("test-occlude", false);
  if (test_occlude) {
    _true_cull_handler->record_object(viz, _internal_trav);
  } else {
    _internal_cull_handler->record_object(viz, _internal_trav);
  }

  PT(OcclusionQueryContext) query = gsg->end_occlusion_query();

  if (show_occlusion) {
    // Show the results of the occlusion.  To do this, we need to get the
    // results of the query immediately.  This will stall the pipe, but we're
    // rendering a debug effect, so we don't mind too much.
    int num_fragments = query->get_num_fragments();
    show_results(num_fragments, geom, net_transform, internal_transform);
  }

  return query;
}

/**
 * Draws a visualization of the results of occlusion test for a particular
 * bounding volume.
 */
void PipeOcclusionCullTraverser::
show_results(int num_fragments, const Geom *geom,
             const TransformState *net_transform,
             const TransformState *internal_transform) {
  LColor color;
  if (num_fragments == 0) {
    // Magenta: culled
    color.set(0.8f, 0.0f, 1.0f, 0.4f);
  } else {
    // Yellow: visible
    color.set(1.0f, 1.0f, 0.5f, 0.4f);
  }

  CPT(RenderState) state = RenderState::make
    (DepthWriteAttrib::make(DepthWriteAttrib::M_off),
     DepthTestAttrib::make(DepthTestAttrib::M_less),
     TransparencyAttrib::make(TransparencyAttrib::M_alpha),
     ColorAttrib::make_flat(color));

  CullableObject *internal_viz =
    new CullableObject(geom, state, internal_transform);
  _internal_cull_handler->record_object(internal_viz, _internal_trav);

  // Also render the viz in the main scene.
  internal_transform = get_scene()->get_cs_world_transform()->compose(net_transform);
  CullableObject *main_viz =
    new CullableObject(geom, state, internal_transform);
  _true_cull_handler->record_object(main_viz, this);
}
