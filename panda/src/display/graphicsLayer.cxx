// Filename: graphicsLayer.cxx
// Created by:  drose (18Apr00)
// 
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "graphicsLayer.h"
#include "graphicsChannel.h"
#include "graphicsWindow.h"
#include "config_display.h"

#include <notify.h>

#include <algorithm>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle GraphicsLayer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GraphicsLayer::
GraphicsLayer() {
  _channel = NULL;
  _is_active = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GraphicsLayer::
GraphicsLayer(GraphicsChannel *channel)
  : _channel(channel) 
{
  _is_active = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::Copy Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GraphicsLayer::
GraphicsLayer(const GraphicsLayer&) {
  display_cat.error()
    << "GraphicsLayers should never be copied" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::Copy Assignment Operator
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GraphicsLayer &GraphicsLayer::
operator=(const GraphicsLayer&) {
  display_cat.error()
  << "GraphicsLayers should never be assigned" << endl;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GraphicsLayer::
~GraphicsLayer() {
  // We don't have to destruct our child display regions explicitly,
  // since they are all reference-counted and will go away when their
  // pointers do.  However, we do need to zero out their pointers to
  // us.
  DisplayRegions::iterator dri;
  for (dri = _display_regions.begin(); 
       dri != _display_regions.end();
       ++dri) {
    (*dri)->_layer = NULL;
  }

  // We don't need to remove ourself from the channel's list of
  // layers.  We must have already been removed, or we wouldn't be
  // destructing!
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::make_display_region
//       Access: Public
//  Description: Creates a new DisplayRegion that covers the entire
//               layer.
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsLayer::
make_display_region() {
  PT(DisplayRegion) dr = new DisplayRegion(this);
  const GraphicsWindow *win = get_window();
  if (win != (GraphicsWindow *)NULL) {
    dr->compute_pixels(win->get_width(), win->get_height());
  }
  _display_regions.push_back(dr);
  return dr;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::make_display_region
//       Access: Public
//  Description: Creates a new DisplayRegion that covers the indicated
//               sub-rectangle within the layer.
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsLayer::
make_display_region(float l, float r, float b, float t) {
  nassertr(this != (GraphicsLayer *)NULL, NULL);
  PT(DisplayRegion) dr = new DisplayRegion(this, l, r, b, t);
  const GraphicsWindow *win = get_window();
  if (win != (GraphicsWindow *)NULL) {
    dr->compute_pixels(win->get_width(), win->get_height());
  }
  _display_regions.push_back(dr);
  return dr;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::get_num_drs
//       Access: Public
//  Description: Returns the number of DisplayRegions associated with
//               the layer.
////////////////////////////////////////////////////////////////////
int GraphicsLayer::
get_num_drs() const {
  return _display_regions.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::get_dr
//       Access: Public
//  Description: Returns the nth DisplayRegion associated with the
//               layer.
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsLayer::
get_dr(int index) const {
  nassertr(index >= 0 && index < (int)_display_regions.size(), NULL);
  return _display_regions[index];
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::remove_dr
//       Access: Public
//  Description: Removes (and possibly deletes) the nth DisplayRegion
//               associated with the layer.  All subsequent index
//               numbers will shift down one.
////////////////////////////////////////////////////////////////////
void GraphicsLayer::
remove_dr(int index) {
  nassertv(index >= 0 && index < (int)_display_regions.size());
  _display_regions[index]->_layer = NULL;
  _display_regions.erase(_display_regions.begin() + index);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::remove_dr
//       Access: Public
//  Description: Removes (and possibly deletes) the indicated
//               DisplayRegion associated with the layer.  All
//               subsequent index numbers will shift down one.
//               Returns true if the DisplayRegion was removed, false
//               if it was not a member of the layer.
////////////////////////////////////////////////////////////////////
bool GraphicsLayer::
remove_dr(DisplayRegion *display_region) {
  // For whatever reason, VC++ considers == ambiguous unless we
  // compare it to a PT(DisplayRegion) instead of a DisplayRegion*.
  PT(DisplayRegion) ptdr = display_region;
  DisplayRegions::iterator dri =
    find(_display_regions.begin(), _display_regions.end(), ptdr);
  if (dri != _display_regions.end()) {
    display_region->_layer = NULL;
    _display_regions.erase(dri);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::get_window
//       Access: Public
//  Description: Returns the GraphicsWindow that this layer is
//               ultimately associated with, or NULL if no window is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsWindow *GraphicsLayer::
get_window() const {
  nassertr(this != (GraphicsLayer *)NULL, NULL);
  return (_channel != (GraphicsChannel *)NULL) ? _channel->get_window() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::get_pipe
//       Access: Public
//  Description: Returns the GraphicsPipe that this layer is
//               ultimately associated with, or NULL if no pipe is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsPipe *GraphicsLayer::
get_pipe() const {
  return (_channel != (GraphicsChannel *)NULL) ? _channel->get_pipe() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::channel_resized
//       Access: Public, Virtual
//  Description: This is called whenever the parent channel has been
//               resized; it should do whatever needs to be done to
//               adjust the layer to account for it.
////////////////////////////////////////////////////////////////////
void GraphicsLayer::
channel_resized(int x, int y) {
  // Since a layer always fills the whole channel, when the channel
  // resizes so does the layer, by the same amount.
  DisplayRegions::iterator dri;
  for (dri = _display_regions.begin(); 
       dri != _display_regions.end();
       ++dri) {
    (*dri)->compute_pixels(x, y);
  }
}
