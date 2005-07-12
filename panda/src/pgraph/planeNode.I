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
 
////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::set_priority
//       Access: Published
//  Description: Changes the relative importance of this PlaneNode
//               (when it is used as a clip plane) relative to the
//               other clip planes that are applied simultaneously.
//
//               The priority number is used to decide which of the
//               requested clip planes are to be activated when more
//               clip planes are requested than the hardware will
//               support.  The highest-priority n planes are selected
//               for rendering.
//
//               This is similar to TextureStage::set_priority().
////////////////////////////////////////////////////////////////////
INLINE void PlaneNode::
set_priority(int priority) {
  _priority = priority;

  // Update the global flag to indicate that all ClipPlaneAttribs in
  // the world must now re-sort their lists.
  _sort_seq++;
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::get_priority
//       Access: Published
//  Description: Returns the priority associated with this clip
//               plane.  See set_priority().
////////////////////////////////////////////////////////////////////
INLINE int PlaneNode::
get_priority() const {
  return _priority;
}

////////////////////////////////////////////////////////////////////
//     Function: PlaneNode::get_sort_seq
//       Access: Public, Static
//  Description: Returns a global sequence number that is incremented
//               any time any PlaneNode in the world changes sort
//               or priority.  This is used by ClipPlaneAttrib to
//               determine when it is necessary to re-sort its
//               internal array of stages.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq PlaneNode::
get_sort_seq() {
  return _sort_seq;
}

