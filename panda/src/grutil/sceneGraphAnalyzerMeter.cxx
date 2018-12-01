/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneGraphAnalyzerMeter.cxx
 * @author pratt
 * @date 2007-02-14
 */

#include "sceneGraphAnalyzerMeter.h"
#include "camera.h"
#include "displayRegion.h"
#include "orthographicLens.h"
#include "config_grutil.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "pStatTimer.h"
#include "omniBoundingVolume.h"
#include <stdio.h>  // For sprintf/snprintf

PStatCollector SceneGraphAnalyzerMeter::_show_analyzer_pcollector("*:Show scene graph analysis");

TypeHandle SceneGraphAnalyzerMeter::_type_handle;

/**
 *
 */
SceneGraphAnalyzerMeter::
SceneGraphAnalyzerMeter(const std::string &name, PandaNode *node) :
  TextNode(name),
  _last_aspect_ratio(-1) {

  set_cull_callback();

  // Don't do frustum culling, as the text will always be in view.
  set_bounds(new OmniBoundingVolume());
  set_final(true);

  Thread *current_thread = Thread::get_current_thread();

  _update_interval = scene_graph_analyzer_meter_update_interval;
  _last_update = 0.0f;
  _node = node;
  _clock_object = ClockObject::get_global_clock();

  set_align(A_left);
  set_transform(LMatrix4::scale_mat(scene_graph_analyzer_meter_scale) *
                LMatrix4::translate_mat(LVector3::rfu(scene_graph_analyzer_meter_side_margins * scene_graph_analyzer_meter_scale, 0.0f, -scene_graph_analyzer_meter_scale)));
  set_card_color(0.0f, 0.0f, 0.0f, 0.4);
  set_card_as_margin(scene_graph_analyzer_meter_side_margins, scene_graph_analyzer_meter_side_margins, 0.1f, 0.0f);
  set_usage_hint(Geom::UH_client);

  do_update(current_thread);
}

/**
 *
 */
SceneGraphAnalyzerMeter::
~SceneGraphAnalyzerMeter() {
  clear_window();
}

/**
 * Sets up the frame rate meter to create a DisplayRegion to render itself
 * into the indicated window.
 */
void SceneGraphAnalyzerMeter::
setup_window(GraphicsOutput *window) {
  clear_window();

  _window = window;

  _root = NodePath("scene_graph_analyzer_root");
  _root.attach_new_node(this);

  CPT(RenderAttrib) dt = DepthTestAttrib::make(DepthTestAttrib::M_none);
  CPT(RenderAttrib) dw = DepthWriteAttrib::make(DepthWriteAttrib::M_off);
  _root.node()->set_attrib(dt, 1);
  _root.node()->set_attrib(dw, 1);
  _root.set_material_off(1);
  _root.set_two_sided(1, 1);

  // If we don't set this explicitly, Panda will cause it to be rendered
  // in a back-to-front cull bin, which will cause the bounding volume
  // to be computed unnecessarily.  Saves a little bit of overhead.
  _root.set_bin("unsorted", 0);

  // Create a display region that covers the entire window.
  _display_region = _window->make_display_region();
  _display_region->set_sort(scene_graph_analyzer_meter_layer_sort);

  // Finally, we need a camera to associate with the display region.
  PT(Camera) camera = new Camera("scene_graph_analyzer_camera");
  NodePath camera_np = _root.attach_new_node(camera);

  PT(Lens) lens = new OrthographicLens;

  // We choose these values such that we can place the text against (0, 0).
  static const PN_stdfloat left = 0.0f;
  static const PN_stdfloat right = 2.0f;
  static const PN_stdfloat bottom = -2.0f;
  static const PN_stdfloat top = 0.0f;
  lens->set_film_size(right - left, top - bottom);
  lens->set_film_offset((right + left) * 0.5, (top + bottom) * 0.5);
  lens->set_near_far(-1000, 1000);

  camera->set_lens(lens);
  camera->set_scene(_root);
  _display_region->set_camera(camera_np);
}

/**
 * Undoes the effect of a previous call to setup_window().
 */
void SceneGraphAnalyzerMeter::
clear_window() {
  if (_window != nullptr) {
    _window->remove_display_region(_display_region);
    _window = nullptr;
    _display_region = nullptr;
  }
  _root = NodePath();
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool SceneGraphAnalyzerMeter::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  Thread *current_thread = trav->get_current_thread();

  // Statistics
  PStatTimer timer(_show_analyzer_pcollector, current_thread);

  // This is probably a good time to check if the aspect ratio on the window
  // has changed.
  int width = _display_region->get_pixel_width();
  int height = _display_region->get_pixel_height();
  PN_stdfloat aspect_ratio = 1;
  if (width != 0 && height != 0) {
    aspect_ratio = (PN_stdfloat)height / (PN_stdfloat)width;
  }

  // Scale the transform by the calculated aspect ratio.
  if (aspect_ratio != _last_aspect_ratio) {
    _aspect_ratio_transform = TransformState::make_scale(LVecBase3(aspect_ratio, 1, 1));
    _last_aspect_ratio = aspect_ratio;
  }
  data._net_transform = data._net_transform->compose(_aspect_ratio_transform);

  // Check to see if it's time to update.
  double now = _clock_object->get_frame_time(current_thread);
  double elapsed = now - _last_update;
  if (elapsed < 0.0 || elapsed >= _update_interval) {
    do_update(current_thread);
  }

  return TextNode::cull_callback(trav, data);
}

/**
 * Resets the text according to the current frame rate.
 */
void SceneGraphAnalyzerMeter::
do_update(Thread *current_thread) {
  _last_update = _clock_object->get_frame_time(current_thread);

  _scene_graph_analyzer.clear();
  _scene_graph_analyzer.add_node( _node );

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];

  const char *pattern = "Nodes: %d\n"
                        "Instances: %d\n"
                        "Transforms: %d\n"
                        "Nodes with Attribs: %d\n"
                        "GeomNodes: %d\n"
                        "Geoms: %d\n"
                        "GeomVertexDatas: %d\n"
                        "Vertices: %d\n"
                        "Normals: %d\n"
                        "TexCoords: %d\n"
                        "Tris: %d\n"
                        "Lines: %d\n"
                        "Points: %d\n"
                        "Texture memory: %.1f KB\n";

#if defined(WIN32_VC) || defined(WIN64_VC)
  _snprintf(buffer, buffer_size, pattern,
          _scene_graph_analyzer.get_num_nodes(),
          _scene_graph_analyzer.get_num_instances(),
          _scene_graph_analyzer.get_num_transforms(),
          _scene_graph_analyzer.get_num_nodes_with_attribs(),
          _scene_graph_analyzer.get_num_geom_nodes(),
          _scene_graph_analyzer.get_num_geoms(),
          _scene_graph_analyzer.get_num_geom_vertex_datas(),
          _scene_graph_analyzer.get_num_vertices(),
          _scene_graph_analyzer.get_num_normals(),
          _scene_graph_analyzer.get_num_texcoords(),
          _scene_graph_analyzer.get_num_tris(),
          _scene_graph_analyzer.get_num_lines(),
          _scene_graph_analyzer.get_num_points(),
          _scene_graph_analyzer.get_texture_bytes()/1024.0);
#else
  snprintf(buffer, buffer_size, pattern,
           _scene_graph_analyzer.get_num_nodes(),
           _scene_graph_analyzer.get_num_instances(),
           _scene_graph_analyzer.get_num_transforms(),
           _scene_graph_analyzer.get_num_nodes_with_attribs(),
           _scene_graph_analyzer.get_num_geom_nodes(),
           _scene_graph_analyzer.get_num_geoms(),
           _scene_graph_analyzer.get_num_geom_vertex_datas(),
           _scene_graph_analyzer.get_num_vertices(),
           _scene_graph_analyzer.get_num_normals(),
           _scene_graph_analyzer.get_num_texcoords(),
           _scene_graph_analyzer.get_num_tris(),
           _scene_graph_analyzer.get_num_lines(),
           _scene_graph_analyzer.get_num_points(),
           _scene_graph_analyzer.get_texture_bytes()/1024.0);
#endif
  nassertv(strlen(buffer) < buffer_size);

  if (get_text() == buffer) {
    // Never mind; the data hasn't changed.
    return;
  }

  set_text(buffer);
}
