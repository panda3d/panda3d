// Filename: graphicsChannel.cxx
// Created by:  mike (09Jan97)
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

#include "graphicsChannel.h"
#include "graphicsWindow.h"
#include "graphicsLayer.h"
#include "config_display.h"
#include "mutexHolder.h"

#include "pmap.h"

TypeHandle GraphicsChannel::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsChannel::
GraphicsChannel() {
  _window = NULL;
  _is_active = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::Constructor
//       Access: Public
//  Description: This is public just so derived window types can
//               easily call it.  Don't call it directly; instead, use
//               GraphicsWindow::get_channel() to get a channel.
////////////////////////////////////////////////////////////////////
GraphicsChannel::
GraphicsChannel(GraphicsWindow *window)
  : _window(window)
{
  _is_active = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::Copy Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GraphicsChannel::
GraphicsChannel(const GraphicsChannel &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::Copy Assignment Operator
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GraphicsChannel::
operator = (const GraphicsChannel &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsChannel::
~GraphicsChannel() {
  // We don't have to destruct our child display regions explicitly,
  // since they are all reference-counted and will go away when their
  // pointers do.  However, we do need to zero out their pointers to
  // us.
  GraphicsLayers::iterator li;
  for (li = _layers.begin();
       li != _layers.end();
       ++li) {
    GraphicsLayer *layer = (*li);
    MutexHolder holder(layer->_lock);
    layer->_channel = NULL;
  }

  // We don't need to remove ourself from the windows's list of
  // channels.  We must have already been removed, or we wouldn't be
  // destructing!
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::make_layer
//       Access: Published
//  Description: Creates a new GraphicsLayer, associated with the
//               window, with the indicated sort value.  The sort
//               value is an arbitrary integer, and may be zero or
//               negative.  The graphics layers are rendered in order
//               from lower sort value to higher sort value; within
//               layers of the same sort value, they are ordered in
//               sequence from the first added to the last added.
////////////////////////////////////////////////////////////////////
GraphicsLayer *GraphicsChannel::
make_layer(int sort) {
  MutexHolder holder(_lock);
  PT(GraphicsLayer) layer = new GraphicsLayer(this, sort);
  _layers.insert(layer);

  return layer;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::remove_layer
//       Access: Published
//  Description: Removes the indicated GraphicsLayer.  Returns true if
//               it was successfully removed, false if it was not a
//               member of the channel in the first place.
////////////////////////////////////////////////////////////////////
bool GraphicsChannel::
remove_layer(GraphicsLayer *layer) {
  MutexHolder holder(_lock);
  GraphicsLayers::iterator li = find_layer(layer);

  if (li == _layers.end()) {
    return false;
  }

  _layers.erase(li); 
  win_display_regions_changed();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::move_layer
//       Access: Published
//  Description: Reinserts the indicated layer into the list with the
//               indicated sort value.  The layer will be rendered
//               last among all of the previously-added layers with
//               the same sort value.  If the new sort value is the
//               same as the previous sort value, the layer will be
//               moved to the end of the list of layers with this sort
//               value.
//
//               Returns true if the layer is successfully moved,
//               false if it is not a member of this channel.
////////////////////////////////////////////////////////////////////
bool GraphicsChannel::
move_layer(GraphicsLayer *layer, int sort) {
  MutexHolder holder(_lock);

  GraphicsLayers::iterator li = find_layer(layer);
  if (li == _layers.end()) {
    return false;
  }

  PT(GraphicsLayer) hold_layer = layer;
  _layers.erase(li);
  layer->_sort = sort;
  _layers.insert(layer);

  win_display_regions_changed();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::get_num_layers
//       Access: Published
//  Description: Returns the number of layers currently associated
//               with the channel.
////////////////////////////////////////////////////////////////////
int GraphicsChannel::
get_num_layers() const {
  MutexHolder holder(_lock);
  return _layers.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::get_layer
//       Access: Published
//  Description: Returns the nth layer associated with the channel.
//               Walking through this list from 0 to (get_num_layers()
//               - 1) will retrieve all of the layers in the order in
//               which they will be rendered.  It is therefore invalid
//               to call make_layer(), remove_layer(), or move_layer()
//               while traversing this list.
////////////////////////////////////////////////////////////////////
GraphicsLayer *GraphicsChannel::
get_layer(int index) const {
  MutexHolder holder(_lock);
  nassertr(index >= 0 && index < (int)_layers.size(), NULL);
  return _layers[index];
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::get_window
//       Access: Published
//  Description: Returns the GraphicsWindow that this channel is
//               associated with.  It is possible that the
//               GraphicsWindow might have been deleted while an
//               outstanding PT(GraphicsChannel) prevented all of its
//               children channels from also being deleted; in this
//               unlikely case, get_window() may return NULL.
////////////////////////////////////////////////////////////////////
GraphicsWindow *GraphicsChannel::
get_window() const {
  MutexHolder holder(_lock);
  return _window;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::get_pipe
//       Access: Published
//  Description: Returns the GraphicsPipe that this channel is
//               ultimately associated with, or NULL if no pipe is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsPipe *GraphicsChannel::
get_pipe() const {
  MutexHolder holder(_lock);
  return (_window != (GraphicsWindow *)NULL) ? _window->get_pipe() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::set_active
//       Access: Published
//  Description: Sets the active flag on the channel.  If the channel
//               is marked as inactive, nothing will be rendered.
////////////////////////////////////////////////////////////////////
void GraphicsChannel::
set_active(bool active) {
  MutexHolder holder(_lock);
  if (active != _is_active) {
    _is_active = active;
    win_display_regions_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::window_resized
//       Access: Public, Virtual
//  Description: This is called whenever the parent window has been
//               resized; it should do whatever needs to be done to
//               adjust the channel to account for it.
////////////////////////////////////////////////////////////////////
void GraphicsChannel::
window_resized(int x_size, int y_size) {
  // By default, a normal GraphicsChannel fills the whole window, and
  // so when the window resizes so does the channel, by the same
  // amount.
  MutexHolder holder(_lock);
  GraphicsLayers::iterator li;
  for (li = _layers.begin();
       li != _layers.end();
       ++li) {
    (*li)->channel_resized(x_size, y_size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::win_display_regions_changed
//       Access: Private
//  Description: Intended to be called when the active state on a
//               nested channel or layer or display region changes,
//               forcing the window to recompute its list of active
//               display regions.  It is assumed the lock is already
//               held.
////////////////////////////////////////////////////////////////////
void GraphicsChannel::
win_display_regions_changed() {
  if (_window != (GraphicsWindow *)NULL) {
    _window->win_display_regions_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::find_layer
//       Access: Private
//  Description: Returns the iterator corresponding to the indicated
//               layer, or _layers.end() if the layer is not part of
//               this channel.
////////////////////////////////////////////////////////////////////
GraphicsChannel::GraphicsLayers::iterator GraphicsChannel::
find_layer(GraphicsLayer *layer) {
  GraphicsLayers::iterator li = _layers.lower_bound(layer);
  while (li != _layers.end() && (*li) != layer) {
    if (*layer < *(*li)) {
      // The layer was not found.
      return _layers.end();
    }
    ++li;
  }

  return li;
}
