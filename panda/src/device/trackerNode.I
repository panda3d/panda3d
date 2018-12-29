/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file trackerNode.I
 * @author drose
 * @date 2002-03-12
 */

/**
 * Returns true if the TrackerNode is valid and connected to a server, false
 * otherwise.
 */
INLINE bool TrackerNode::
is_valid() const {
  return (!_tracker.is_null()/* && _tracker->is_connected()*/);
}

/**
 * Returns the current position of the tracker, if it is available.
 */
INLINE const LPoint3 &TrackerNode::
get_pos() const {
  return _data.get_pos();
}

/**
 * Returns the current orientation of the tracker, if it is available.
 */
INLINE const LOrientation &TrackerNode::
get_orient() const {
  return _data.get_orient();
}

/**
 * Returns the time of the tracker's last update.
 */
INLINE double TrackerNode::
get_time() const {
  return _data.get_time();
}

/**
 * True if this data comes with timestamps.
 */
INLINE bool TrackerNode::
has_time() const {
  return _data.has_time();
}

/**
 * Returns the current position and orientation of the tracker, as a combined
 * matrix.
 */
INLINE const LMatrix4 &TrackerNode::
get_transform() const {
  return _mat;
}


/**
 * Specifies the coordinate system that the tracker associated with this node
 * will operate in.  Normally, this is set from the ClientBase that's used to
 * create the TrackerNode, so it should not need to be set on an individual
 * tracker basis.
 */
INLINE void TrackerNode::
set_tracker_coordinate_system(CoordinateSystem cs) {
  _tracker_cs = cs;
  if (_tracker_cs == CS_default) {
    _tracker_cs = get_default_coordinate_system();
  }
}

/**
 * Returns the coordinate system that the tracker associated with this node
 * will operate in.
 */
INLINE CoordinateSystem TrackerNode::
get_tracker_coordinate_system() const {
  return _tracker_cs;
}

/**
 * Specifies the coordinate system that the TrackerNode will convert its
 * transform into for passing down the data graph.  Normally, this is
 * CS_default.
 */
INLINE void TrackerNode::
set_graph_coordinate_system(CoordinateSystem cs) {
  _graph_cs = cs;
  if (_graph_cs == CS_default) {
    _graph_cs = get_default_coordinate_system();
  }
}

/**
 * Returns the coordinate system that the TrackerNode will convert its
 * transform into for passing down the data graph.  Normally, this is
 * CS_default.
 */
INLINE CoordinateSystem TrackerNode::
get_graph_coordinate_system() const {
  return _graph_cs;
}
