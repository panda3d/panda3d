// Filename: planeNode.cxx
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
#include "planeNode.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle PlaneNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *PlaneNode::
make_copy() const {
  return new PlaneNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: set_plane
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void PlaneNode::set_plane(const Planef& plane)
{
  _plane = plane;
}

////////////////////////////////////////////////////////////////////
//     Function: get_plane
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
const Planef& PlaneNode::get_plane(void) const
{
  return _plane;
}
