// Filename: displayRegion.cxx
// Created by:  cary (10Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "displayRegion.h"
#include "graphicsLayer.h"
#include "graphicsChannel.h"
#include "graphicsWindow.h"
#include "config_display.h"


////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DisplayRegion::
DisplayRegion(GraphicsLayer *layer) :
  _l(0.), _r(1.), _b(0.), _t(1.), 
  _layer(layer),
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
	      const float r, const float b, const float t)
  : _l(l), _r(r), _b(b), _t(t),
    _layer(layer),
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
  _layer((GraphicsLayer *)0L),
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
  display_cat.error()
    << "DisplayRegions should not be copied" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Copy Assignment Operator
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
DisplayRegion &DisplayRegion::
operator=(const DisplayRegion&) {
  display_cat.error()
  << "DisplayRegions should not be assigned" << endl;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DisplayRegion::
~DisplayRegion() {
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
set_camera(const PT(Camera) &camera) {
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
