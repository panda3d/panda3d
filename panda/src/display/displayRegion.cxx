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


#include "graphicsLayer.h"
#include "graphicsChannel.h"
#include "graphicsOutput.h"
#include "config_display.h"
#include "displayRegion.h"
#include "camera.h"
#include "dcast.h"
#include "mutexHolder.h"


////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(GraphicsLayer *layer) :
  _l(0.), _r(1.), _b(0.), _t(1.),
  _layer(layer),
  _camera_node((Camera *)NULL),
  _active(true)
{
  compute_pixels();
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(GraphicsLayer *layer, const float l,
              const float r, const float b, const float t) :
  _l(l), _r(r), _b(b), _t(t),
  _layer(layer),
  _camera_node((Camera *)NULL),
  _active(true)
{
  compute_pixels();
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Constructor
//       Access: Public
//  Description: This constructor makes a DisplayRegion that is not
//               associated with any particular layer; this is
//               typically for rendering a temporary pass.
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(int xsize, int ysize) :
  _l(0.), _r(1.), _b(0.), _t(1.),
  _pl(0), _pr(xsize), _pb(0), _pt(ysize), _pbi(ysize), _pti(0),
  _layer((GraphicsLayer *)NULL),
  _camera_node((Camera *)NULL),
  _active(true)
{
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
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_dimensions
//       Access: Published
//  Description: Retrieves the coordinates of the DisplayRegion's
//               rectangle within its GraphicsLayer.  These numbers
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
//               rectangle within its GraphicsLayer.  This number
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
//               rectangle within its GraphicsLayer.  This number
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
//               the rectangle within its GraphicsLayer.  This 
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
//               rectangle within its GraphicsLayer.  This number
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
//     Function: DisplayRegion::get_layer
//       Access: Published
//  Description: Returns the layer associated with this particular
//               DisplayRegion, or NULL if no layer is associated
//               (or if the layer was deleted).
////////////////////////////////////////////////////////////////////
GraphicsLayer *DisplayRegion::
get_layer() const {
  MutexHolder holder(_lock);
  return _layer;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_channel
//       Access: Published
//  Description: Returns the GraphicsChannel that this DisplayRegion is
//               ultimately associated with, or NULL if no channel is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsChannel *DisplayRegion::
get_channel() const {
  MutexHolder holder(_lock);
  return (_layer != (GraphicsLayer *)NULL) ? _layer->get_channel() : NULL;
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
  return (_layer != (GraphicsLayer *)NULL) ? _layer->get_window() : NULL;
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
  return (_layer != (GraphicsLayer *)NULL) ? _layer->get_pipe() : NULL;
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
const NodePath &DisplayRegion::
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
//     Function: DisplayRegion::compute_pixels
//       Access: Published
//  Description: Computes the pixel locations of the DisplayRegion
//               within its layer.  The DisplayRegion will request the
//               size from the window.
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
//               within its layer, given the size of the layer in
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
//               its layer, in pixels.
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
//               its layer, as the pixel location of its bottom-left
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
  if (_layer != (GraphicsLayer *)NULL) {
    _layer->win_display_regions_changed();
  }
}
