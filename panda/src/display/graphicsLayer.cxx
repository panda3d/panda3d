// Filename: graphicsLayer.cxx
// Created by:  drose (18Apr00)
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
#include "graphicsOutput.h"
#include "config_display.h"
#include "notify.h"
#include "mutexHolder.h"

#include <algorithm>


TypeHandle GraphicsLayer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::Constructor
//       Access: Private
//  Description: Use GraphicsChannel::make_layer() to make a new
//               layer.
////////////////////////////////////////////////////////////////////
GraphicsLayer::
GraphicsLayer() {
  _channel = NULL;
  _sort = 0;
  _is_active = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::Constructor
//       Access: Public
//  Description: Use GraphicsChannel::make_layer() to make a new
//               layer.
////////////////////////////////////////////////////////////////////
GraphicsLayer::
GraphicsLayer(GraphicsChannel *channel, int sort) :
  _channel(channel),
  _sort(sort)
{
  _is_active = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::Copy Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsLayer::
GraphicsLayer(const GraphicsLayer &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::Copy Assignment Operator
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsLayer::
operator = (const GraphicsLayer &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::Destructor
//       Access: Published
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
    DisplayRegion *dr = (*dri);
    MutexHolder holder(dr->_lock);
    dr->_layer = NULL;
  }
  win_display_regions_changed();

  // We don't need to remove ourself from the channel's list of
  // layers.  We must have already been removed, or we wouldn't be
  // destructing!
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::make_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the entire
//               layer.
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsLayer::
make_display_region() {
  PT(DisplayRegion) dr = new DisplayRegion(this);

  MutexHolder holder(_lock);
  _display_regions.push_back(dr);
  win_display_regions_changed();
  return dr;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::make_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the indicated
//               sub-rectangle within the layer.
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsLayer::
make_display_region(float l, float r, float b, float t) {
  nassertr(this != (GraphicsLayer *)NULL, NULL);
  PT(DisplayRegion) dr = new DisplayRegion(this, l, r, b, t);

  MutexHolder holder(_lock);
  _display_regions.push_back(dr);
  win_display_regions_changed();
  return dr;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::get_num_drs
//       Access: Published
//  Description: Returns the number of DisplayRegions associated with
//               the layer.
////////////////////////////////////////////////////////////////////
int GraphicsLayer::
get_num_drs() const {
  MutexHolder holder(_lock);
  return _display_regions.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::get_dr
//       Access: Published
//  Description: Returns the nth DisplayRegion associated with the
//               layer.  This might return NULL if another thread has
//               recently removed a DisplayRegion.
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsLayer::
get_dr(int index) const {
  MutexHolder holder(_lock);
  if (index >= 0 && index < (int)_display_regions.size()) {
    return _display_regions[index];
  } else {
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::remove_dr
//       Access: Published
//  Description: Removes (and possibly deletes) the nth DisplayRegion
//               associated with the layer.  All subsequent index
//               numbers will shift down one.
////////////////////////////////////////////////////////////////////
void GraphicsLayer::
remove_dr(int index) {
  MutexHolder holder(_lock);
  nassertv(index >= 0 && index < (int)_display_regions.size());
  _display_regions[index]->_layer = NULL;
  _display_regions.erase(_display_regions.begin() + index);
  win_display_regions_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::remove_dr
//       Access: Published
//  Description: Removes (and possibly deletes) the indicated
//               DisplayRegion associated with the layer.  All
//               subsequent index numbers will shift down one.
//               Returns true if the DisplayRegion was removed, false
//               if it was not a member of the layer.
////////////////////////////////////////////////////////////////////
bool GraphicsLayer::
remove_dr(DisplayRegion *display_region) {
  MutexHolder holder(_lock);
  PT(DisplayRegion) ptdr = display_region;
  DisplayRegions::iterator dri =
    find(_display_regions.begin(), _display_regions.end(), ptdr);
  if (dri != _display_regions.end()) {
    display_region->_layer = NULL;
    _display_regions.erase(dri);
    win_display_regions_changed();
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::get_channel
//       Access: Published
//  Description: Returns the GraphicsChannel that this layer is
//               associated with.  It is possible that the
//               GraphicsChannel might have been deleted while an
//               outstanding PT(GraphicsLayer) prevented all of its
//               children layers from also being deleted; in this
//               unlikely case, get_channel() may return NULL.
////////////////////////////////////////////////////////////////////
GraphicsChannel *GraphicsLayer::
get_channel() const {
  MutexHolder holder(_lock);
  return _channel;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::get_window
//       Access: Published
//  Description: Returns the GraphicsOutput that this layer is
//               ultimately associated with, or NULL if no window is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsOutput *GraphicsLayer::
get_window() const {
  MutexHolder holder(_lock);
  return (_channel != (GraphicsChannel *)NULL) ? _channel->get_window() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::get_pipe
//       Access: Published
//  Description: Returns the GraphicsPipe that this layer is
//               ultimately associated with, or NULL if no pipe is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsPipe *GraphicsLayer::
get_pipe() const {
  MutexHolder holder(_lock);
  return (_channel != (GraphicsChannel *)NULL) ? _channel->get_pipe() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::set_active
//       Access: Published
//  Description: Sets the active flag on the layer.  If the layer
//               is marked as inactive, nothing will be rendered.
////////////////////////////////////////////////////////////////////
void GraphicsLayer::
set_active(bool active) {
  MutexHolder holder(_lock);
  if (active != _is_active) {
    _is_active = active;
    win_display_regions_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::set_sort
//       Access: Published
//  Description: Changes the sort parameter on the layer.  If the sort
//               parameter is changed, the layer will be reordered
//               among the other layers on the same channel.  If the
//               sort parameter is not changed, the layer will remain
//               in the same sort position.  (This is different from
//               GraphicsChannel::move_layer(), which will reorder the
//               layer even if the sort parameter does not change.)
////////////////////////////////////////////////////////////////////
void GraphicsLayer::
set_sort(int sort) {
  if (sort != _sort) {
    _channel->move_layer(this, sort);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::channel_resized
//       Access: Public
//  Description: This is called whenever the parent channel has been
//               resized; it should do whatever needs to be done to
//               adjust the layer to account for it.
////////////////////////////////////////////////////////////////////
void GraphicsLayer::
channel_resized(int x, int y) {
  MutexHolder holder(_lock);
  // Since a layer always fills the whole channel, when the channel
  // resizes so does the layer, by the same amount.
  DisplayRegions::iterator dri;
  for (dri = _display_regions.begin();
       dri != _display_regions.end();
       ++dri) {
    (*dri)->compute_pixels(x, y);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::win_display_regions_changed
//       Access: Private
//  Description: Intended to be called when the active state on a
//               nested channel or layer or display region changes,
//               forcing the window to recompute its list of active
//               display regions.  It is assumed the lock is already
//               held.
////////////////////////////////////////////////////////////////////
void GraphicsLayer::
win_display_regions_changed() {
  if (_channel != (GraphicsChannel *)NULL) {
    _channel->win_display_regions_changed();
  }
}
