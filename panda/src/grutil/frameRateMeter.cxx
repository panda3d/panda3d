/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file frameRateMeter.cxx
 * @author drose
 * @date 2003-12-23
 */

#include "frameRateMeter.h"
#include "camera.h"
#include "displayRegion.h"
#include "orthographicLens.h"
#include "clockObject.h"
#include "config_grutil.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "pStatTimer.h"
#include "omniBoundingVolume.h"
#include <stdio.h>  // For sprintf/snprintf

PStatCollector FrameRateMeter::_show_fps_pcollector("*:Show fps");

TypeHandle FrameRateMeter::_type_handle;

/**
 *
 */
FrameRateMeter::
FrameRateMeter(const std::string &name) :
  TextNode(name),
  _last_aspect_ratio(-1) {

  set_cull_callback();

  // Don't do frustum culling, as the text will always be in view.
  set_bounds(new OmniBoundingVolume());
  set_final(true);

  Thread *current_thread = Thread::get_current_thread();

  _show_milliseconds = frame_rate_meter_milliseconds;
  if (_show_milliseconds) {
    _text_pattern = frame_rate_meter_ms_text_pattern;
  } else {
    _text_pattern = frame_rate_meter_text_pattern;
  }

  _update_interval = frame_rate_meter_update_interval;
  _last_update = 0.0f;
  _clock_object = ClockObject::get_global_clock();

  // The top of the visible frame is 80% of the line height, based on the
  // calculation within TextAssembler.
  PN_stdfloat height = 1.0f;
  TextFont *font = get_font();
  if (font != nullptr){
    height = font->get_line_height() * 0.8;
  }

  set_align(A_right);
  set_transform(LMatrix4::scale_mat(frame_rate_meter_scale) *
                LMatrix4::translate_mat(LVector3::rfu(-frame_rate_meter_side_margins * frame_rate_meter_scale, 0.0f, -frame_rate_meter_scale * height)));
  set_card_color(0.0f, 0.0f, 0.0f, 0.4);
  set_card_as_margin(frame_rate_meter_side_margins, frame_rate_meter_side_margins, 0.1f, 0.0f);
  // set_usage_hint(Geom::UH_client);

  do_update(current_thread);
}

/**
 *
 */
FrameRateMeter::
~FrameRateMeter() {
  clear_window();
}

/**
 * Sets up the frame rate meter to create a DisplayRegion to render itself
 * into the indicated window.
 */
void FrameRateMeter::
setup_window(GraphicsOutput *window) {
  clear_window();

  _window = window;

  _root = NodePath("frame_rate_root");
  _root.attach_new_node(this);

  CPT(RenderAttrib) dt = DepthTestAttrib::make(DepthTestAttrib::M_none);
  CPT(RenderAttrib) dw = DepthWriteAttrib::make(DepthWriteAttrib::M_off);
  _root.node()->set_attrib(dt, 1);
  _root.node()->set_attrib(dw, 1);
  _root.set_material_off(1);
  _root.set_two_sided(1, 1);

  // If we don't set this explicitly, Panda will cause it to be rendered in a
  // back-to-front cull bin, which will cause the bounding volume to be
  // computed unnecessarily.  Saves a little bit of overhead.
  _root.set_bin("unsorted", 0);

  // Create a display region that covers the entire window.
  _display_region = _window->make_mono_display_region();
  _display_region->set_sort(frame_rate_meter_layer_sort);

  // Finally, we need a camera to associate with the display region.
  PT(Camera) camera = new Camera("frame_rate_camera");
  NodePath camera_np = _root.attach_new_node(camera);

  PT(Lens) lens = new OrthographicLens;

  // We choose these values such that we can place the text against (0, 0).
  static const PN_stdfloat left = -2.0f;
  static const PN_stdfloat right = 0.0f;
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
void FrameRateMeter::
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
bool FrameRateMeter::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // This triggers when you try to parent a frame rate meter into the scene
  // graph yourself.  Instead, use setup_window().
  nassertr(_display_region != nullptr, false);

  Thread *current_thread = trav->get_current_thread();

  // Statistics
  PStatTimer timer(_show_fps_pcollector, current_thread);

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
void FrameRateMeter::
do_update(Thread *current_thread) {
  _last_update = _clock_object->get_frame_time(current_thread);

  double value = _clock_object->get_average_frame_rate(current_thread);
  double deviation = _clock_object->calc_frame_rate_deviation(current_thread);

  if (_show_milliseconds) {
    value = 1000.0 / value;
    deviation = 1000.0 / deviation;
  }

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];
#if defined(WIN32_VC) || defined(WIN64_VC)
  // Windows doesn't define snprintf().  Hope we don't overflow.
  sprintf(buffer, _text_pattern.c_str(), value, deviation);
#else
  snprintf(buffer, buffer_size, _text_pattern.c_str(), value, deviation);
#endif
  nassertv(strlen(buffer) < buffer_size);

  if (get_text() == buffer) {
    // Never mind; the frame rate hasn't changed.
    return;
  }

  set_text(buffer);
}
