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
//     Function: GraphicsLayer::Comparison operator
//       Access: Published
//  Description: Sorts the layers in order by sort parameter only.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsLayer::
operator < (const GraphicsLayer &other) const {
  return _sort < other._sort;
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsLayer::get_sort
//       Access: Published
//  Description: Returns the sort parameter associated with the layer.
//               This defines the order in which this layer is
//               rendered relative to the other layers on the channel.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsLayer::
get_sort() const {
  return _sort;
}
