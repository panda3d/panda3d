// Filename: graphicsLayer.I
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
INLINE GraphicsChannel *GraphicsLayer::
get_channel() const {
  return _channel;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::set_active
//       Access: Published
//  Description: Sets the active flag on the layer.  If the layer
//               is marked as inactive, nothing will be rendered.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsLayer::
set_active(bool active) {
  if (active != _is_active) {
    _is_active = active;
    win_display_regions_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::is_active
//       Access: Published
//  Description: Returns the active flag on the layer.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsLayer::
is_active() const {
  return _is_active;
}

