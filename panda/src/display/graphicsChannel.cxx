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
#include "config_display.h"

#include <map>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle GraphicsChannel::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::Constructor
//       Access: Public
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
//  Description:
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
GraphicsChannel(const GraphicsChannel&) {
  display_cat.error()
    << "GraphicsChannels should never be copied" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::Copy Assignment Operator
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GraphicsChannel &GraphicsChannel::
operator=(const GraphicsChannel&) {
  display_cat.error()
  << "GraphicsChannels should never be assigned" << endl;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::Destructor
//       Access: Public
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
    (*li)->_channel = NULL;
  }

  // We don't need to remove ourself from the windows's list of
  // channels.  We must have already been removed, or we wouldn't be
  // destructing!
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::make_layer
//       Access: Public
//  Description: Creates a new GraphicsLayer, associated with the
//               window, at the indicated index position.  If the
//               index position negative or past the end of the array,
//               the end of the array is assumed.  The layers will be
//               rendered on top of each other, in increasing order by
//               index, from back to front.
////////////////////////////////////////////////////////////////////
GraphicsLayer *GraphicsChannel::
make_layer(int index) {
  PT(GraphicsLayer) layer = new GraphicsLayer(this);
  if (index < 0 || index >= (int)_layers.size()) {
    _layers.push_back(layer);
  } else {
    _layers.insert(_layers.begin() + index, layer);
  }
  return layer;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::get_num_layers
//       Access: Public
//  Description: Returns the number of layers currently associated
//               with the channel.
////////////////////////////////////////////////////////////////////
int GraphicsChannel::
get_num_layers() const {
  return _layers.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::get_layer
//       Access: Public
//  Description: Returns the nth layer associated with the channel.
////////////////////////////////////////////////////////////////////
GraphicsLayer *GraphicsChannel::
get_layer(int index) const {
  nassertr(index >= 0 && index < (int)_layers.size(), NULL);
  return _layers[index];
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::move_layer
//       Access: Public
//  Description: Changes the ordering of the layers so that the
//               indicated layer will move to the indicated position.
//               If to_index is negative or past the end of the array,
//               the end of the array is assumed.
////////////////////////////////////////////////////////////////////
void GraphicsChannel::
move_layer(int from_index, int to_index) {
  nassertv(from_index >= 0 && from_index < (int)_layers.size());
  PT(GraphicsLayer) layer = _layers[from_index];

  if (to_index < 0 || to_index >= (int)_layers.size()) {
    _layers.erase(_layers.begin() + from_index);
    _layers.push_back(layer);

  } else if (to_index > from_index) {
    // Move the layer later in the list.
    _layers.insert(_layers.begin() + to_index, layer);
    _layers.erase(_layers.begin() + from_index);

  } else if (to_index < from_index) {
    // Move the layer earlier in the list.
    _layers.erase(_layers.begin() + from_index);
    _layers.insert(_layers.begin() + to_index, layer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::remove_layer
//       Access: Public
//  Description: Removes the nth layer.  This changes the numbers of
//               all subsequent layers.
////////////////////////////////////////////////////////////////////
void GraphicsChannel::
remove_layer(int index) {
  nassertv(index >= 0 && index < (int)_layers.size());
  _layers.erase(_layers.begin() + index);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::get_window
//       Access: Public
//  Description: Returns the GraphicsWindow that this channel is
//               associated with.  It is possible that the
//               GraphicsWindow might have been deleted while an
//               outstanding PT(GraphicsChannel) prevented all of its
//               children channels from also being deleted; in this
//               unlikely case, get_window() may return NULL.
////////////////////////////////////////////////////////////////////
GraphicsWindow *GraphicsChannel::
get_window() const {
  return _window;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::get_pipe
//       Access: Public
//  Description: Returns the GraphicsPipe that this channel is
//               ultimately associated with, or NULL if no pipe is
//               associated.
////////////////////////////////////////////////////////////////////
GraphicsPipe *GraphicsChannel::
get_pipe() const {
  return (_window != (GraphicsWindow *)NULL) ? _window->get_pipe() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::window_resized
//       Access: Public, Virtual
//  Description: This is called whenever the parent window has been
//               resized; it should do whatever needs to be done to
//               adjust the channel to account for it.
////////////////////////////////////////////////////////////////////
void GraphicsChannel::
window_resized(int x, int y) {
  // By default, a normal GraphicsChannel fills the whole window, and
  // so when the window resizes so does the channel, by the same
  // amount.
  GraphicsLayers::iterator li;
  for (li = _layers.begin();
       li != _layers.end();
       ++li) {
    (*li)->channel_resized(x, y);
  }
}
