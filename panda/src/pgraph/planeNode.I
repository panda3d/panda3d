// Filename: planeNode.I
// Created by:  drose (11Jul02)
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


////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PlaneNode::CData::
CData() {
  // The default plane (perpendicular to the Z-axis) is used until
  // another one is specified explicitly.
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PlaneNode::CData::
CData(const PlaneNode::CData &copy) :
  _plane(copy._plane)
{
}

////////////////////////////////////////////////////////////////////
//     Function: set_plane
//       Access: Public
//  Description: Sets the particular plane represented by the
//               PlaneNode.
////////////////////////////////////////////////////////////////////
INLINE void PlaneNode::
set_plane(const Planef &plane) {
  CDWriter cdata(_cycler);
  cdata->_plane = plane;
}

////////////////////////////////////////////////////////////////////
//     Function: get_plane
//       Access: Public
//  Description: Returns the plane represented by the PlaneNode.
////////////////////////////////////////////////////////////////////
INLINE const Planef &PlaneNode::
get_plane() const {
  CDReader cdata(_cycler);
  return cdata->_plane;
}
