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
#include "mutexHolder.h"
#include "pnmImage.h"


////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(GraphicsOutput *window) :
  _l(0.), _r(1.), _b(0.), _t(1.),
  _window(window),
  _camera_node((Camera *)NULL),
  _active(true),
  _sort(0)
{
  _draw_buffer_type = window->get_draw_buffer_type();
  compute_pixels();
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(GraphicsOutput *window, const float l,
              const float r, const float b, const float t) :
  _l(l), _r(r), _b(b), _t(t),
  _window(window),
  _camera_node((Camera *)NULL),
  _active(true),
  _sort(0)
{
  _draw_buffer_type = window->get_draw_buffer_type();
  compute_pixels();
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Copy Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(const DisplayRegion&) {
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
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
~DisplayRegion() {
  set_camera(NodePath());

  // The window pointer should already have been cleared by the time
  // the DisplayRegion destructs (since the GraphicsOutput class keeps
  // a reference count on the DisplayRegion).
  nassertv(_window == (GraphicsOutput *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_dimensions
//       Access: Published
//  Description: Retrieves the coordinates of the DisplayRegion's
//               rectangle within its GraphicsOutput.  These numbers
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
void DisplayRegion::
get_dimensions(float &l, float &r, float &b, float &t) const {
  MutexHolder holder(_lock);
  l = _l;
  r = _r;
  b = _b;
  t = _t;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_left
//       Access: Published
//  Description: Retrieves the x coordinate of the left edge of the
//               rectangle within its GraphicsOutput.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
float DisplayRegion::
get_left() const {
  MutexHolder holder(_lock);
  return _l;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_right
//       Access: Published
//  Description: Retrieves the x coordinate of the right edge of the
//               rectangle within its GraphicsOutput.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
float DisplayRegion::
get_right() const {
  MutexHolder holder(_lock);
  return _r;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_bottom
//       Access: Published
//  Description: Retrieves the y coordinate of the bottom edge of 
//               the rectangle within its GraphicsOutput.  This 
//               number will be in the range [0..1].
////////////////////////////////////////////////////////////////////
float DisplayRegion::
get_bottom() const {
  MutexHolder holder(_lock);
  return _b;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_top
//       Access: Published
//  Description: Retrieves the y coordinate of the top edge of the
//               rectangle within its GraphicsOutput.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
float DisplayRegion::
get_top() const {
  MutexHolder holder(_lock);
  return _t;
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
  MutexHolder holder(_lock);
  _l = l;
  _r = r;
  _b = b;
  _t = t;

  const GraphicsOutput *win = get_window();
  if (win != (GraphicsOutput *)NULL && win->has_size()) {
    do_compute_pixels(win->get_x_size(), win->get_y_size());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_window
//       Access: Published
//  Description: Returns the GraphicsOutput that this DisplayRegion is
//               ultimately associated with, or NULL if no window is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsOutput *DisplayRegion::
get_window() const {
  MutexHolder holder(_lock);
  return _window;
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
  MutexHolder holder(_lock);
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
  MutexHolder holder(_lock);
  Camera *camera_node = (Camera *)NULL;
  if (!camera.is_empty()) {
    DCAST_INTO_V(camera_node, camera.node());
  }

  if (camera_node != _camera_node) {
    if (_camera_node != (Camera *)NULL) {
      // We need to tell the old camera we're not using him anymore.
      _camera_node->remove_display_region(this);
    }
    _camera_node = camera_node;
    if (_camera_node != (Camera *)NULL) {
      // Now tell the new camera we are using him.
      _camera_node->add_display_region(this);
    }
  }

  _camera = camera;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_camera
//       Access: Published
//  Description: Returns the camera associated with this
//               DisplayRegion, or an empty NodePath if no camera is
//               associated.
////////////////////////////////////////////////////////////////////
NodePath DisplayRegion::
get_camera() const {
  MutexHolder holder(_lock);
  return _camera;
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
  MutexHolder holder(_lock);
  if (active != _active) {
    _active = active;
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
  MutexHolder holder(_lock);
  if (sort != _sort) {
    _sort = sort;
    win_display_regions_changed();
  }
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
  const GraphicsOutput *win = get_window();
  if (win != (GraphicsOutput *)NULL) {
    MutexHolder holder(_lock);

    do_compute_pixels(win->get_x_size(), win->get_y_size());
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
  MutexHolder holder(_lock);
  do_compute_pixels(x_size, y_size);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pixels
//       Access: Published
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its window, in pixels.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
get_pixels(int &pl, int &pr, int &pb, int &pt) const {
  MutexHolder holder(_lock);
  pl = _pl;
  pr = _pr;
  pb = _pb;
  pt = _pt;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_region_pixels
//       Access: Published
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its window, as the pixel location of its bottom-left
//               corner, along with a pixel width and height.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
get_region_pixels(int &xo, int &yo, int &w, int &h) const {
  MutexHolder holder(_lock);
  xo = _pl;
  yo = _pb;
  w = _pr - _pl;
  h = _pt - _pb;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_region_pixels_i
//       Access: Published
//  Description: Similar to get_region_pixels(), but returns the upper
//               left corner, and the pixel numbers are numbered from
//               the top-left corner down, in the DirectX way of
//               things.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
get_region_pixels_i(int &xo, int &yo, int &w, int &h) const {
  MutexHolder holder(_lock);
  xo = _pl;
  yo = _pti;
  w = _pr - _pl;
  h = _pbi - _pti;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pixel_width
//       Access: Published
//  Description: Returns the width of the DisplayRegion in pixels.
////////////////////////////////////////////////////////////////////
int DisplayRegion::
get_pixel_width() const {
  MutexHolder holder(_lock);
  return _pr - _pl;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pixel_height
//       Access: Published
//  Description: Returns the height of the DisplayRegion in pixels.
////////////////////////////////////////////////////////////////////
int DisplayRegion::
get_pixel_height() const {
  MutexHolder holder(_lock);
  return _pt - _pb;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void DisplayRegion::
output(ostream &out) const {
  MutexHolder holder(_lock);
  out << "DisplayRegion(" << _l << " " << _r << " " << _b << " " << _t
      << ")=pixels(" << _pl << " " << _pr << " " << _pb << " " << _pt
      << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::save_screenshot_default
//       Access: Published
//  Description: Saves a screenshot of the region to a default
//               filename, and returns the filename, or empty string
//               if the screenshot failed.  The default filename is
//               generated from the supplied prefix and from the
//               Config variable screenshot-filename, which contains
//               the following strings:
//
//                 %~p - the supplied prefix
//                 %~f - the frame count
//                 %~e - the value of screenshot-extension
//                 All other % strings in strftime().
////////////////////////////////////////////////////////////////////
Filename DisplayRegion::
save_screenshot_default(const string &prefix) {
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

  Filename filename = filename_strm.str();
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
save_screenshot(const Filename &filename) {
  PNMImage image;
  if (!get_screenshot(image)) {
    return false;
  }

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
  GraphicsOutput *window = get_window();
  nassertr(window != (GraphicsOutput *)NULL, false);
  
  GraphicsStateGuardian *gsg = window->get_gsg();
  nassertr(gsg != (GraphicsStateGuardian *)NULL, false);
  
  window->make_current();

  int components = 3;
  Texture::Format format = Texture::F_rgb;

  if ((gsg->get_properties().get_frame_buffer_mode() & FrameBufferProperties::FM_alpha) != 0) {
    components = 4;
    format = Texture::F_rgba;
  }

  // Create a temporary texture to receive the framebuffer image.
  PT(Texture) tex = new Texture;

  RenderBuffer buffer = gsg->get_render_buffer(get_screenshot_buffer_type());
  if (!gsg->framebuffer_copy_to_ram(tex, this, buffer)) {
    return false;
  }

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
do_compute_pixels(int x_size, int y_size) {
  if (display_cat.is_debug()) {
    display_cat.debug()
      << "DisplayRegion::do_compute_pixels(" << x_size << ", " << y_size << ")\n";
  }

  _pl = int((_l * x_size) + 0.5);
  _pr = int((_r * x_size) + 0.5);

  const GraphicsOutput *win = get_window();
  nassertv(win != (GraphicsOutput *)NULL);
  if (win->get_inverted()) {
    // The window is inverted; compute the DisplayRegion accordingly.
    _pb = int(((1.0f - _t) * y_size) + 0.5);
    _pt = int(((1.0f - _b) * y_size) + 0.5);
    _pbi = int((_t * y_size) + 0.5);
    _pti = int((_b * y_size) + 0.5);

  } else {
    // The window is normal.
    _pb = int((_b * y_size) + 0.5);
    _pt = int((_t * y_size) + 0.5);
    _pbi = int(((1.0f - _b) * y_size) + 0.5);
    _pti = int(((1.0f - _t) * y_size) + 0.5);
  }
}
