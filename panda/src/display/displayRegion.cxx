// Filename: displayRegion.cxx
// Created by:  cary (10Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "displayRegion.h"
#include "stereoDisplayRegion.h"
#include "graphicsEngine.h"
#include "graphicsOutput.h"
#include "config_display.h"
#include "texture.h"
#include "camera.h"
#include "dcast.h"
#include "pnmImage.h"

#include <time.h>

TypeHandle DisplayRegion::_type_handle;
TypeHandle DisplayRegionPipelineReader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(GraphicsOutput *window, const LVecBase4 &dimensions) :
  _window(window),
  _incomplete_render(true),
  _texture_reload_priority(0),
  _cull_region_pcollector("Cull:Invalid"),
  _draw_region_pcollector("Draw:Invalid")
{
  _screenshot_buffer_type = window->get_draw_buffer_type();
  _draw_buffer_type = window->get_draw_buffer_type();
  set_dimensions(dimensions);
  compute_pixels_all_stages();

  _window->add_display_region(this);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Copy Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(const DisplayRegion &copy) : 
  _window(NULL),
  _cull_region_pcollector("Cull:Invalid"),
  _draw_region_pcollector("Draw:Invalid")
{
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Copy Assignment Operator
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void DisplayRegion::
operator = (const DisplayRegion&) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
~DisplayRegion() {
  cleanup();

  // The window pointer should already have been cleared by the time
  // the DisplayRegion destructs (since the GraphicsOutput class keeps
  // a reference count on the DisplayRegion).
  nassertv(_window == (GraphicsOutput *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::cleanup
//       Access: Public
//  Description: Cleans up some pointers associated with the
//               DisplayRegion to help reduce the chance of memory
//               leaks due to circular reference counts.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
cleanup() {
  set_camera(NodePath());

  CDCullWriter cdata(_cycler_cull, true);
  cdata->_cull_result = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_lens_index
//       Access: Published
//  Description: Sets the lens index, allows for multiple lenses to
//               be attached to a camera.  This is useful for a 
//               variety of setups, such as fish eye rendering.
//               The default is 0.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_lens_index(int index) {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);
  CDWriter cdata(_cycler);
  cdata->_lens_index = index;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_dimensions
//       Access: Published, Virtual
//  Description: Changes the portion of the framebuffer this
//               DisplayRegion corresponds to.  The parameters range
//               from 0 to 1, where 0,0 is the lower left corner and
//               1,1 is the upper right; (0, 1, 0, 1) represents the
//               whole screen.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_dimensions(const LVecBase4 &dimensions) {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);
  CDWriter cdata(_cycler);

  cdata->_dimensions = dimensions;

  if (_window != (GraphicsOutput *)NULL && _window->has_size()) {
    do_compute_pixels(_window->get_fb_x_size(), _window->get_fb_y_size(), cdata);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pipe
//       Access: Published
//  Description: Returns the GraphicsPipe that this DisplayRegion is
//               ultimately associated with, or NULL if no pipe is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsPipe *DisplayRegion::
get_pipe() const {
  return (_window != (GraphicsOutput *)NULL) ? _window->get_pipe() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::is_stereo
//       Access: Published, Virtual
//  Description: Returns true if this is a StereoDisplayRegion, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool DisplayRegion::
is_stereo() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_camera
//       Access: Published, Virtual
//  Description: Sets the camera that is associated with this
//               DisplayRegion.  There is a one-to-many association
//               between cameras and DisplayRegions; one camera may be
//               shared by multiple DisplayRegions.
//
//               The camera is actually set via a NodePath, which
//               clarifies which instance of the camera (if there
//               happen to be multiple instances) we should use.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_camera(const NodePath &camera) {
  int pipeline_stage = Thread::get_current_pipeline_stage();

  // We allow set_camera(NodePath()) to happen in cleanup(), which can
  // be called from any pipeline stage.
  nassertv(pipeline_stage == 0 || camera.is_empty());
  CDStageWriter cdata(_cycler, 0);

  Camera *camera_node = (Camera *)NULL;
  if (!camera.is_empty()) {
    DCAST_INTO_V(camera_node, camera.node());
  }

  if (camera_node != cdata->_camera_node) {
    // Note that these operations on the DisplayRegion are not
    // pipelined: they operate across all pipeline stages.  Since we
    // have already asserted we are running in pipeline stage 0, no
    // problem.
    if (cdata->_camera_node != (Camera *)NULL) {
      // We need to tell the old camera we're not using him anymore.
      cdata->_camera_node->remove_display_region(this);
    }
    cdata->_camera_node = camera_node;
    if (cdata->_camera_node != (Camera *)NULL) {
      // Now tell the new camera we are using him.
      cdata->_camera_node->add_display_region(this);
    }
  }

  cdata->_camera = camera;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_active
//       Access: Published, Virtual
//  Description: Sets the active flag associated with the
//               DisplayRegion.  If the DisplayRegion is marked
//               inactive, nothing is rendered.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_active(bool active) {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);
  CDLockedReader cdata(_cycler);

  if (active != cdata->_active) {
    CDWriter cdataw(_cycler, cdata);
    cdataw->_active = active;
    win_display_regions_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_sort
//       Access: Published, Virtual
//  Description: Sets the sort value associated with the
//               DisplayRegion.  Within a window, DisplayRegions will
//               be rendered in order from the lowest sort value to
//               the highest.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_sort(int sort) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDLockedReader cdata(_cycler);

  if (sort != cdata->_sort) {
    CDWriter cdataw(_cycler, cdata);
    cdataw->_sort = sort;
    win_display_regions_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_stereo_channel
//       Access: Published, Virtual
//  Description: Specifies whether the DisplayRegion represents the
//               left or right channel of a stereo pair, or whether it
//               is a normal, monocular image.  This automatically
//               adjusts the lens that is used to render to this
//               DisplayRegion to its left or right eye, according to
//               the lens's stereo properties.
//
//               When the DisplayRegion is attached to a stereo window
//               (one for which is_stereo() returns true), this also
//               specifies which physical channel the DisplayRegion
//               renders to.
//
//               Normally you would create at least two DisplayRegions
//               for a stereo window, one for each of the left and
//               right channels.  The two DisplayRegions may share the
//               same camera (and thus the same lens); this parameter
//               is used to control the exact properties of the lens
//               when it is used to render into this DisplayRegion.
//
//               Also see the StereoDisplayRegion, which automates
//               managing a pair of left/right DisplayRegions.
//
//               An ordinary DisplayRegion may be set to SC_mono,
//               SC_left, or SC_right.  You may set SC_stereo only on
//               a StereoDisplayRegion.
//
//               This call also resets tex_view_offset to its default
//               value, which is 0 for the left eye or 1 for the right
//               eye of a stereo display region, or 0 for a mono
//               display region.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_stereo_channel(Lens::StereoChannel stereo_channel) {
  nassertv(is_stereo() || stereo_channel != Lens::SC_stereo);

  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDWriter cdata(_cycler);
  cdata->_stereo_channel = stereo_channel;
  cdata->_tex_view_offset = (stereo_channel == Lens::SC_right) ? 1 : 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_tex_view_offset
//       Access: Published, Virtual
//  Description: Sets the current texture view offset for this
//               DisplayRegion.  This is normally set to zero.  If
//               nonzero, it is used to select a particular view of
//               any multiview textures that are rendered within this
//               DisplayRegion.
//
//               For a StereoDisplayRegion, this is normally 0 for the
//               left eye, and 1 for the right eye, to support stereo
//               textures.  This is set automatically when you call
//               set_stereo_channel().
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_tex_view_offset(int tex_view_offset) {
  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDWriter cdata(_cycler);
  cdata->_tex_view_offset = tex_view_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_incomplete_render
//       Access: Published, Virtual
//  Description: Sets the incomplete_render flag.  When this is
//               true, the frame will be rendered even if some of the
//               geometry or textures in the scene are not available
//               (e.g. they have been temporarily paged out).  When
//               this is false, the frame will be held up while this
//               data is reloaded.
//
//               This flag may also be set on the
//               GraphicsStateGuardian.  It will be considered true
//               for a given DisplayRegion only if it is true on both
//               the GSG and on the DisplayRegion.
//
//               See GraphicsStateGuardian::set_incomplete_render()
//               for more detail.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_incomplete_render(bool incomplete_render) {
  _incomplete_render = incomplete_render;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_texture_reload_priority
//       Access: Published, Virtual
//  Description: Specifies an integer priority which is assigned to
//               any asynchronous texture reload requests spawned
//               while processing this DisplayRegion.  This controls
//               which textures are loaded first when multiple
//               textures need to be reloaded at once; it also
//               controls the relative priority between asynchronous
//               texture loads and asynchronous model or animation
//               loads.
//
//               Specifying a larger number here makes the textures
//               rendered by this DisplayRegion load up first.  This
//               may be particularly useful to do, for instance, for
//               the DisplayRegion that renders the gui.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_texture_reload_priority(int texture_reload_priority) {
  _texture_reload_priority = texture_reload_priority;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_cull_traverser
//       Access: Published, Virtual
//  Description: Specifies the CullTraverser that will be used to draw
//               the contents of this DisplayRegion.  Normally the
//               default CullTraverser is sufficient, but this may be
//               changed to change the default cull behavior.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_cull_traverser(CullTraverser *trav) {
  _trav = trav;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_cull_traverser
//       Access: Published
//  Description: Returns the CullTraverser that will be used to draw
//               the contents of this DisplayRegion.
////////////////////////////////////////////////////////////////////
CullTraverser *DisplayRegion::
get_cull_traverser() {
  if (_trav == (CullTraverser *)NULL) {
    _trav = new CullTraverser;
  }
  return _trav;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_target_tex_page
//       Access: Published, Virtual
//  Description: This is a special parameter that is only used when
//               rendering the faces of a cube map or multipage and/or
//               multiview texture.  
//
//               This sets up the DisplayRegion to render to the ith
//               page and jth view of its associated texture(s); the
//               value must be consistent with the range of values
//               availble to the texture.  A normal DisplayRegion that
//               is not associated with any particular page should be
//               set to page -1 and view 0.
//
//               This is particularly useful when rendering cube maps
//               and/or stereo textures.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_target_tex_page(int page) {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);
  CDWriter cdata(_cycler);
  cdata->_target_tex_page = page;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DisplayRegion::
output(ostream &out) const {
  CDReader cdata(_cycler);
  out << "DisplayRegion(" << cdata->_dimensions << ")=pixels(" << cdata->_pl
      << " " << cdata->_pr << " " << cdata->_pb << " " << cdata->_pt
      << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::make_screenshot_filename
//       Access: Published, Static
//  Description: Synthesizes a suitable default filename for passing
//               to save_screenshot().
//
//               The default filename is generated from the supplied
//               prefix and from the Config variable
//               screenshot-filename, which contains the following
//               strings:
//
//                 %~p - the supplied prefix
//                 %~f - the frame count
//                 %~e - the value of screenshot-extension
//                 All other % strings in strftime().
////////////////////////////////////////////////////////////////////
Filename DisplayRegion::
make_screenshot_filename(const string &prefix) {
  time_t now = time(NULL);
  struct tm *ttm = localtime(&now);
  int frame_count = ClockObject::get_global_clock()->get_frame_count();

  static const int buffer_size = 1024;
  char buffer[buffer_size];

  ostringstream filename_strm;

  size_t i = 0;
  while (i < screenshot_filename.length()) {
    char ch1 = screenshot_filename[i++];
    if (ch1 == '%' && i < screenshot_filename.length()) {
      char ch2 = screenshot_filename[i++];
      if (ch2 == '~' && i < screenshot_filename.length()) {
        char ch3 = screenshot_filename[i++];
        switch (ch3) {
        case 'p':
          filename_strm << prefix;
          break;

        case 'f':
          filename_strm << frame_count;
          break;

        case 'e':
          filename_strm << screenshot_extension;
          break;
        }

      } else {
        // Use strftime() to decode the percent code.
        char format[3] = {'%', ch2, '\0'};
        if (strftime(buffer, buffer_size, format, ttm)) {
          for (char *b = buffer; *b != '\0'; b++) {
            switch (*b) {
            case ' ':
            case ':':
            case '/':
              filename_strm << '-';
              break;

            case '\n':
              break;

            default:
              filename_strm << *b;
            }
          }
        }
      }
    } else {
      filename_strm << ch1;
    }
  }

  return Filename(filename_strm.str());
}


////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::save_screenshot_default
//       Access: Published
//  Description: Saves a screenshot of the region to a default
//               filename, and returns the filename, or empty string
//               if the screenshot failed.  The filename is generated
//               by make_screenshot_filename().
////////////////////////////////////////////////////////////////////
Filename DisplayRegion::
save_screenshot_default(const string &prefix) {
  Filename filename = make_screenshot_filename(prefix);
  if (save_screenshot(filename)) {
    return filename;
  }
  return Filename();
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::save_screenshot
//       Access: Published
//  Description: Saves a screenshot of the region to the indicated
//               filename.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DisplayRegion::
save_screenshot(const Filename &filename, const string &image_comment) {
  PNMImage image;
  if (!get_screenshot(image)) {
    return false;
  }

  image.set_comment(image_comment);
  if (!image.write(filename)) {
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_screenshot
//       Access: Published
//  Description: Captures the most-recently rendered image from the
//               framebuffer into the indicated PNMImage.  Returns
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DisplayRegion::
get_screenshot(PNMImage &image) {
  PT(Texture) tex = get_screenshot();

  if (tex == NULL) {
    return false;
  }
  
  if (!tex->store(image)) {
    return false;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_screenshot
//       Access: Published
//  Description: Captures the most-recently rendered image from the
//               framebuffer and returns it as a Texture, or NULL
//               on failure.
////////////////////////////////////////////////////////////////////
PT(Texture) DisplayRegion::
get_screenshot() {
  Thread *current_thread = Thread::get_current_thread();

  GraphicsOutput *window = get_window();
  nassertr(window != (GraphicsOutput *)NULL, NULL);
  
  GraphicsStateGuardian *gsg = window->get_gsg();
  nassertr(gsg != (GraphicsStateGuardian *)NULL, NULL);
  
  if (!window->begin_frame(GraphicsOutput::FM_refresh, current_thread)) {
    return NULL;
  }

  PT(Texture) tex = new Texture;
  
  RenderBuffer buffer = gsg->get_render_buffer(get_screenshot_buffer_type(),
                                               _window->get_fb_properties());
  if (!gsg->framebuffer_copy_to_ram(tex, 0, -1, this, buffer)) {
    return NULL;
  }
  
  window->end_frame(GraphicsOutput::FM_refresh, current_thread);
  
  return tex;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::make_cull_result_graph
//       Access: Published
//  Description: Returns a special scene graph constructed to
//               represent the results of the last frame's cull
//               operation.
//
//               This will be a hierarchy of nodes, one node for each
//               bin, each of which will in term be a parent of a
//               number of GeomNodes, representing the geometry drawn
//               in each bin.
//
//               This is useful mainly for high-level debugging and
//               abstraction tools; it should not be mistaken for the
//               low-level cull result itself, which is constructed
//               and maintained internally.  No such scene graph is
//               normally constructed during the rendering of a frame;
//               this is an artificial construct created for the
//               purpose of making it easy to analyze the results of
//               the cull operation.
////////////////////////////////////////////////////////////////////
PT(PandaNode) DisplayRegion::
make_cull_result_graph() {
  CullResult *cull_result = get_cull_result(Thread::get_current_thread());
  if (cull_result == (CullResult *)NULL) {
    return NULL;
  }
  return cull_result->make_result_graph();
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::compute_pixels
//       Access: Public
//  Description: Computes the pixel locations of the DisplayRegion
//               within its window.  The DisplayRegion will request
//               the size from the window.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
compute_pixels() {
  if (_window != (GraphicsOutput *)NULL) {
    CDWriter cdata(_cycler, false);
    do_compute_pixels(_window->get_fb_x_size(), _window->get_fb_y_size(), 
                      cdata);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::compute_pixels_all_stages
//       Access: Public
//  Description: Computes the pixel locations of the DisplayRegion
//               within its window.  The DisplayRegion will request
//               the size from the window.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
compute_pixels_all_stages() {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);

  if (_window != (GraphicsOutput *)NULL) {
    OPEN_ITERATE_ALL_STAGES(_cycler) {
      CDStageWriter cdata(_cycler, pipeline_stage);
      do_compute_pixels(_window->get_fb_x_size(), _window->get_fb_y_size(), 
                        cdata);
    }
    CLOSE_ITERATE_ALL_STAGES(_cycler);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::compute_pixels
//       Access: Public
//  Description: Computes the pixel locations of the DisplayRegion
//               within its window, given the size of the window in
//               pixels.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
compute_pixels(int x_size, int y_size) {
  CDWriter cdata(_cycler, false);
  do_compute_pixels(x_size, y_size, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::compute_pixels_all_stages
//       Access: Public
//  Description: Performs a compute_pixels() operation for all stages
//               of the pipeline.  This is appropriate, for instance,
//               when a window changes sizes, since this is a global
//               operation; and you want the new window size to be
//               immediately available even to the downstream stages.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
compute_pixels_all_stages(int x_size, int y_size) {
  OPEN_ITERATE_ALL_STAGES(_cycler) {
    CDStageWriter cdata(_cycler, pipeline_stage);
    do_compute_pixels(x_size, y_size, cdata);
  } 
  CLOSE_ITERATE_ALL_STAGES(_cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::supports_pixel_zoom
//       Access: Public, Virtual
//  Description: Returns true if a call to set_pixel_zoom() will be
//               respected, false if it will be ignored.  If this
//               returns false, then get_pixel_factor() will always
//               return 1.0, regardless of what value you specify for
//               set_pixel_zoom().
//
//               This may return false if the underlying renderer
//               doesn't support pixel zooming, or if you have called
//               this on a DisplayRegion that doesn't have both
//               set_clear_color() and set_clear_depth() enabled.
////////////////////////////////////////////////////////////////////
bool DisplayRegion::
supports_pixel_zoom() const {
  if (_window != (GraphicsOutput *)NULL) {
    if (_window->supports_pixel_zoom()) {
      return get_clear_color_active() && get_clear_depth_active();
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::win_display_regions_changed
//       Access: Private
//  Description: Intended to be called when the active state on a
//               nested channel or layer or display region changes,
//               forcing the window to recompute its list of active
//               display regions.  It is assumed the lock is already
//               held.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
win_display_regions_changed() {
  if (_window != (GraphicsOutput *)NULL) {
    _window->win_display_regions_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::do_compute_pixels
//       Access: Private
//  Description: The private implementation of compute_pixels, this
//               assumes that we already have the lock.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
do_compute_pixels(int x_size, int y_size, CData *cdata) {
  if (display_cat.is_debug()) {
    display_cat.debug()
      << "DisplayRegion::do_compute_pixels(" << x_size << ", " << y_size << ")\n";
  }

  int old_w = cdata->_pr - cdata->_pl;
  int old_h = cdata->_pt - cdata->_pb;

  cdata->_pl = int((cdata->_dimensions[0] * x_size) + 0.5);
  cdata->_pr = int((cdata->_dimensions[1] * x_size) + 0.5);

  nassertv(_window != (GraphicsOutput *)NULL);
  if (_window->get_inverted()) {
    // The window is inverted; compute the DisplayRegion accordingly.
    cdata->_pb = int(((1.0f - cdata->_dimensions[3]) * y_size) + 0.5);
    cdata->_pt = int(((1.0f - cdata->_dimensions[2]) * y_size) + 0.5);
    cdata->_pbi = int((cdata->_dimensions[3] * y_size) + 0.5);
    cdata->_pti = int((cdata->_dimensions[2] * y_size) + 0.5);

  } else {
    // The window is normal.
    cdata->_pb = int((cdata->_dimensions[2] * y_size) + 0.5);
    cdata->_pt = int((cdata->_dimensions[3] * y_size) + 0.5);
    cdata->_pbi = int(((1.0f - cdata->_dimensions[2]) * y_size) + 0.5);
    cdata->_pti = int(((1.0f - cdata->_dimensions[3]) * y_size) + 0.5);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_active_index
//       Access: Private
//  Description: This is called by GraphicsOutput to indicate that the
//               index of this DisplayRegion within the window's list
//               of active DisplayRegions might have changed.  The
//               index number will be -1 if the DisplayRegion is not
//               active.
//
//               This is primarily intended only for updating the
//               PStatCollector name appropriately.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_active_index(int index) {
#ifdef DO_PSTATS
  ostringstream strm;
  strm << "dr_" << index;
  string name = strm.str();

  _cull_region_pcollector = PStatCollector(_window->get_cull_window_pcollector(), name);
  _draw_region_pcollector = PStatCollector(_window->get_draw_window_pcollector(), name);
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::do_cull
//       Access: Protected, Virtual
//  Description: Performs a cull traversal.  The default
//               implementation simply calls GraphicsEngine::do_cull.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
        GraphicsStateGuardian *gsg, Thread *current_thread) {

  GraphicsEngine::do_cull(cull_handler, scene_setup, gsg, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::CData::
CData() :
  _dimensions(0.0f, 1.0f, 0.0f, 1.0f),
  _pl(0), _pr(0), _pb(0), _pt(0),
  _pbi(0), _pti(0), _lens_index(0),
  _camera_node((Camera *)NULL),
  _active(true),
  _sort(0),
  _stereo_channel(Lens::SC_mono),
  _tex_view_offset(0),
  _target_tex_page(-1)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::CData::
CData(const DisplayRegion::CData &copy) :
  _dimensions(copy._dimensions),
  _pl(copy._pl),
  _pr(copy._pr),
  _pb(copy._pb),
  _pt(copy._pt),
  _pbi(copy._pbi),
  _pti(copy._pti),
  _lens_index(copy._lens_index),
  _camera(copy._camera),
  _camera_node(copy._camera_node),
  _active(copy._active),
  _sort(copy._sort),
  _stereo_channel(copy._stereo_channel),
  _tex_view_offset(copy._tex_view_offset),
  _target_tex_page(copy._target_tex_page)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *DisplayRegion::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::CDataCull::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *DisplayRegion::CDataCull::
make_copy() const {
  return new CDataCull(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_pipe
//       Access: Public
//  Description: Returns the GraphicsPipe that this DisplayRegion is
//               ultimately associated with, or NULL if no pipe is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsPipe *DisplayRegionPipelineReader::
get_pipe() const {
  return (_object->_window != (GraphicsOutput *)NULL) ? _object->_window->get_pipe() : NULL;
}
