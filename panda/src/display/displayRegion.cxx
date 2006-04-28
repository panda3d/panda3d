// Filename: displayRegion.cxx
// Created by:  cary (10Feb99)
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

#include "displayRegion.h"
#include "graphicsOutput.h"
#include "config_display.h"
#include "texture.h"
#include "camera.h"
#include "dcast.h"
#include "pnmImage.h"

TypeHandle DisplayRegion::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(GraphicsOutput *window) :
  _window(window),
  _clear_depth_between_eyes(true),
  _cull_region_pcollector("Cull:Invalid"),
  _draw_region_pcollector("Draw:Invalid")
{
  _draw_buffer_type = window->get_draw_buffer_type();
  compute_pixels_all_stages();
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(GraphicsOutput *window, float l, float r, float b, float t) :
  _window(window),
  _clear_depth_between_eyes(true),
  _cull_region_pcollector("Cull:Invalid"),
  _draw_region_pcollector("Draw:Invalid")
{
  _draw_buffer_type = window->get_draw_buffer_type();
  set_dimensions(l, r, b, t);
  compute_pixels_all_stages();
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Copy Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(const DisplayRegion&) : 
  _cull_region_pcollector("Cull:Invalid"),
  _draw_region_pcollector("Draw:Invalid")
{
  nassertv(false);
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
//     Function: DisplayRegion::set_dimensions
//       Access: Published
//  Description: Changes the portion of the framebuffer this
//               DisplayRegion corresponds to.  The parameters range
//               from 0 to 1, where 0,0 is the lower left corner and
//               1,1 is the upper right; (0, 1, 0, 1) represents the
//               whole screen.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_dimensions(float l, float r, float b, float t) {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);
  CDWriter cdata(_cycler);

  cdata->_l = l;
  cdata->_r = r;
  cdata->_b = b;
  cdata->_t = t;

  if (_window != (GraphicsOutput *)NULL && _window->has_size()) {
    do_compute_pixels(_window->get_x_size(), _window->get_y_size(), cdata);
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
//     Function: DisplayRegion::set_camera
//       Access: Published
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
  nassertv(pipeline_stage == 0);
  CDWriter cdata(_cycler);

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
//       Access: Published
//  Description: Sets the active flag associated with the
//               DisplayRegion.  If the DisplayRegion is marked
//               inactive, nothing is rendered.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_active(bool active) {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);
  CDReader cdata(_cycler);

  if (active != cdata->_active) {
    CDWriter cdataw(_cycler, cdata);
    cdataw->_active = active;
    win_display_regions_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_sort
//       Access: Published
//  Description: Sets the sort value associated with the
//               DisplayRegion.  Within a window, DisplayRegions will
//               be rendered in order from the lowest sort value to
//               the highest.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_sort(int sort) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDReader cdata(_cycler);

  if (sort != cdata->_sort) {
    CDWriter cdataw(_cycler, cdata);
    cdataw->_sort = sort;
    win_display_regions_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_stereo_channel
//       Access: Published
//  Description: Specifies whether the DisplayRegion represents the
//               left or right channel of a stereo pair, or whether it
//               is a normal, monocular image.  See
//               set_stereo_channel().
//
//               This controls which direction--to the left or the
//               right--the view from a PerspectiveLens is shifted
//               when it is used to render into this DisplayRegion.
//               Also see Lens::set_interocular_distance() and
//               Lens::set_convergence_distance().
//
//               Normally you would create at least two DisplayRegions
//               for a stereo window, one for each of the left and
//               right channels.  The two DisplayRegions may share the
//               same camera (and thus the same lens); this parameter
//               is used to control the exact properties of the lens
//               when it is used to render into this DisplayRegion.
//
//               When the DisplayRegion is attached to a stereo window
//               (one in which FrameBufferProperties::FM_stereo is
//               set), this also specifies which physical channel the
//               DisplayRegion renders to.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_stereo_channel(Lens::StereoChannel stereo_channel) {
  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDWriter cdata(_cycler);
  cdata->_stereo_channel = stereo_channel;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::compute_pixels
//       Access: Published
//  Description: Computes the pixel locations of the DisplayRegion
//               within its window.  The DisplayRegion will request
//               the size from the window.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
compute_pixels() {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);

  if (_window != (GraphicsOutput *)NULL) {
    CDWriter cdata(_cycler);
    do_compute_pixels(_window->get_x_size(), _window->get_y_size(), 
                      cdata);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::compute_pixels_all_stages
//       Access: Published
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
      do_compute_pixels(_window->get_x_size(), _window->get_y_size(), 
                        cdata);
    }
    CLOSE_ITERATE_ALL_STAGES(_cycler);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::compute_pixels
//       Access: Published
//  Description: Computes the pixel locations of the DisplayRegion
//               within its window, given the size of the window in
//               pixels.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
compute_pixels(int x_size, int y_size) {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);
  CDWriter cdata(_cycler);
  do_compute_pixels(x_size, y_size, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::compute_pixels_all_stages
//       Access: Published
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
//     Function: DisplayRegion::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void DisplayRegion::
output(ostream &out) const {
  CDReader cdata(_cycler);
  out << "DisplayRegion(" << cdata->_l << " " << cdata->_r << " "
      << cdata->_b << " " << cdata->_t << ")=pixels(" << cdata->_pl
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
  Thread *current_thread = Thread::get_current_thread();

  GraphicsOutput *window = get_window();
  nassertr(window != (GraphicsOutput *)NULL, false);
  
  GraphicsStateGuardian *gsg = window->get_gsg();
  nassertr(gsg != (GraphicsStateGuardian *)NULL, false);
  
  if (!window->begin_frame(GraphicsOutput::FM_refresh, current_thread)) {
    return false;
  }

  // Create a temporary texture to receive the framebuffer image.
  PT(Texture) tex = new Texture;
  
  RenderBuffer buffer = gsg->get_render_buffer(get_screenshot_buffer_type(),
                                               _window->get_fb_properties());
  if (!gsg->framebuffer_copy_to_ram(tex, -1, this, buffer)) {
    return false;
  }
  
  window->end_frame(GraphicsOutput::FM_refresh, current_thread);
  
  if (!tex->store(image)) {
    return false;
  }
  
  return true;
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

  cdata->_pl = int((cdata->_l * x_size) + 0.5);
  cdata->_pr = int((cdata->_r * x_size) + 0.5);

  nassertv(_window != (GraphicsOutput *)NULL);
  if (_window->get_inverted()) {
    // The window is inverted; compute the DisplayRegion accordingly.
    cdata->_pb = int(((1.0f - cdata->_t) * y_size) + 0.5);
    cdata->_pt = int(((1.0f - cdata->_b) * y_size) + 0.5);
    cdata->_pbi = int((cdata->_t * y_size) + 0.5);
    cdata->_pti = int((cdata->_b * y_size) + 0.5);

  } else {
    // The window is normal.
    cdata->_pb = int((cdata->_b * y_size) + 0.5);
    cdata->_pt = int((cdata->_t * y_size) + 0.5);
    cdata->_pbi = int(((1.0f - cdata->_b) * y_size) + 0.5);
    cdata->_pti = int(((1.0f - cdata->_t) * y_size) + 0.5);
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
//     Function: DisplayRegion::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::CData::
CData() :
  _l(0.), _r(1.), _b(0.), _t(1.),
  _pl(0), _pr(0), _pb(0), _pt(0),
  _pbi(0), _pti(0),
  _camera_node((Camera *)NULL),
  _active(true),
  _sort(0),
  _stereo_channel(Lens::SC_mono),
  _cube_map_index(-1)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::CData::
CData(const DisplayRegion::CData &copy) :
  _l(copy._l),
  _r(copy._r),
  _b(copy._b),
  _t(copy._t),
  _pl(copy._pl),
  _pr(copy._pr),
  _pb(copy._pb),
  _pt(copy._pt),
  _pbi(copy._pbi),
  _pti(copy._pti),
  _camera(copy._camera),
  _camera_node(copy._camera_node),
  _active(copy._active),
  _sort(copy._sort),
  _stereo_channel(copy._stereo_channel),
  _cube_map_index(copy._cube_map_index)
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
