// Filename: displayRegion.cxx
// Created by:  cary (10Feb99)
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


#include "graphicsLayer.h"
#include "graphicsChannel.h"
#include "graphicsWindow.h"
#include "config_display.h"
#include "displayRegion.h"
#include "qpcamera.h"


////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(GraphicsLayer *layer) :
  _l(0.), _r(1.), _b(0.), _t(1.),
  _layer(layer),
  _camera_node((qpCamera *)NULL),
  _active(true)
{
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
  _camera_node((qpCamera *)NULL),
  _active(true)
{
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
  _pl(0), _pr(xsize), _pb(0), _pt(ysize),
  _layer((GraphicsLayer *)NULL),
  _camera_node((qpCamera *)NULL),
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
  set_qpcamera(qpNodePath());
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_dimensions
//       Access: Public
//  Description: Changes the portion of the framebuffer this
//               DisplayRegion corresponds to.  The parameters range
//               from 0 to 1, where 0,0 is the lower left corner and
//               1,1 is the upper right; (0, 1, 0, 1) represents the
//               whole screen.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_dimensions(float l, float r, float b, float t) {
  _l = l;
  _r = r;
  _b = b;
  _t = t;

  const GraphicsWindow *win = get_window();
  if (win != (GraphicsWindow *)NULL) {
    compute_pixels(win->get_width(), win->get_height());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_channel
//       Access: Public
//  Description: Returns the GraphicsChannel that this DisplayRegion is
//               ultimately associated with, or NULL if no channel is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsChannel *DisplayRegion::
get_channel() const {
  return (_layer != (GraphicsLayer *)NULL) ? _layer->get_channel() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_window
//       Access: Public
//  Description: Returns the GraphicsWindow that this DisplayRegion is
//               ultimately associated with, or NULL if no window is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsWindow *DisplayRegion::
get_window() const {
  return (_layer != (GraphicsLayer *)NULL) ? _layer->get_window() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pipe
//       Access: Public
//  Description: Returns the GraphicsPipe that this DisplayRegion is
//               ultimately associated with, or NULL if no pipe is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsPipe *DisplayRegion::
get_pipe() const {
  return (_layer != (GraphicsLayer *)NULL) ? _layer->get_pipe() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_camera
//       Access: Public
//  Description: Sets the camera that is associated with this
//               DisplayRegion.  Each DisplayRegion may have zero or
//               one cameras associated.  (If it has no camera,
//               nothing is rendered.)  A given camera may be shared
//               between multiple DisplayRegions.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_camera(Camera *camera) {
  if (camera != _camera) {
    if (_camera != (Camera *)NULL) {
      // We need to tell the old camera we're not using him anymore.
      _camera->remove_display_region(this);
    }
    _camera = camera;
    if (_camera != (Camera *)NULL) {
      // Now tell the new camera we are using him.
      _camera->add_display_region(this);
    }
  }
  set_cull_frustum(camera);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_qpcamera
//       Access: Public
//  Description: Sets the camera that is associated with this
//               DisplayRegion.  There is a one-to-one association
//               between cameras and DisplayRegions; if this camera
//               was already associated with a different
//               DisplayRegion, that association is removed.
//
//               The camera is actually set via a qpNodePath, which
//               clarifies which instance of the camera (if there
//               happen to be multiple instances) we should use.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
set_qpcamera(const qpNodePath &camera) {
  qpCamera *camera_node = (qpCamera *)NULL;
  if (!camera.is_empty()) {
    DCAST_INTO_V(camera_node, camera.node());
  }

  if (camera_node != _camera_node) {
    if (_camera_node != (qpCamera *)NULL) {
      // We need to tell the old camera we're not using him anymore.
      _camera_node->remove_display_region(this);
    }
    _camera_node = camera_node;
    if (_camera_node != (qpCamera *)NULL) {
      // Now tell the new camera we are using him.
      _camera_node->add_display_region(this);
    }
  }

  _qpcamera = camera;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DisplayRegion::
output(ostream &out) const {
  out << "DisplayRegion(" << _l << " " << _r << " " << _b << " " << _t
      << ")=pixels(" << _pl << " " << _pr << " " << _pb << " " << _pt
      << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::win_display_regions_changed
//       Access: Public
//  Description: Intended to be called when the active state on a
//               nested channel or layer or display region changes,
//               forcing the window to recompute its list of active
//               display regions.
////////////////////////////////////////////////////////////////////
void DisplayRegion::
win_display_regions_changed() {
  if (_layer != (GraphicsLayer *)NULL) {
    _layer->win_display_regions_changed();
  }
}
