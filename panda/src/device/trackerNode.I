// Filename: trackerNode.I
// Created by:  drose (12Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::is_valid
//       Access: Public
//  Description: Returns true if the TrackerNode is valid and
//               connected to a server, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TrackerNode::
is_valid() const {
  return (_tracker != (ClientTrackerDevice *)NULL) && _tracker->is_connected();
}


////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::get_pos
//       Access: Public
//  Description: Returns the current position of the tracker, if it is
//               available.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3 &TrackerNode::
get_pos() const {
  return _data.get_pos();
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::get_orient
//       Access: Public
//  Description: Returns the current orientation of the tracker, if it
//               is available.
////////////////////////////////////////////////////////////////////
INLINE const LOrientation &TrackerNode::
get_orient() const {
  return _data.get_orient();
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::get_time
//       Access: Public
//  Description: Returns the time of the tracker's last update.
////////////////////////////////////////////////////////////////////
INLINE double TrackerNode::
get_time() const {
  return _data.get_time();
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::has_time
//       Access: Public
//  Description: True if this data comes with timestamps.
////////////////////////////////////////////////////////////////////
INLINE bool TrackerNode::
has_time() const {
  return _data.has_time();
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::get_transform
//       Access: Public
//  Description: Returns the current position and orientation of the
//               tracker, as a combined matrix.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4 &TrackerNode::
get_transform() const {
  return _mat;
}


////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::set_tracker_coordinate_system
//       Access: Published
//  Description: Specifies the coordinate system that the tracker
//               associated with this node will operate in.  Normally,
//               this is set from the ClientBase that's used to create
//               the TrackerNode, so it should not need to be set on
//               an individual tracker basis.
////////////////////////////////////////////////////////////////////
INLINE void TrackerNode::
set_tracker_coordinate_system(CoordinateSystem cs) {
  _tracker_cs = cs;
  if (_tracker_cs == CS_default) {
    _tracker_cs = get_default_coordinate_system();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::get_tracker_coordinate_system
//       Access: Published
//  Description: Returns the coordinate system that the tracker
//               associated with this node will operate in.
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem TrackerNode::
get_tracker_coordinate_system() const {
  return _tracker_cs;
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::set_graph_coordinate_system
//       Access: Published
//  Description: Specifies the coordinate system that the TrackerNode
//               will convert its transform into for passing down the
//               data graph.  Normally, this is CS_default.
////////////////////////////////////////////////////////////////////
INLINE void TrackerNode::
set_graph_coordinate_system(CoordinateSystem cs) {
  _graph_cs = cs;
  if (_graph_cs == CS_default) {
    _graph_cs = get_default_coordinate_system();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::get_graph_coordinate_system
//       Access: Published
//  Description: Returns the coordinate system that the TrackerNode
//               will convert its transform into for passing down the
//               data graph.  Normally, this is CS_default.
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem TrackerNode::
get_graph_coordinate_system() const {
  return _graph_cs;
}
