// Filename: trackerNode.I
// Created by:  drose (25Jan01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::is_valid
//       Access: Public
//  Description: Returns true if the TrackerNode is valid and
//               connected to a server, false otherwise.
////////////////////////////////////////////////////////////////////
bool TrackerNode::
is_valid() const {
  return (_tracker != (ClientTrackerDevice *)NULL) && _tracker->is_connected();
}


////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::get_pos
//       Access: Public
//  Description: Returns the current position of the tracker, if it is
//               available.
////////////////////////////////////////////////////////////////////
const LPoint3f &TrackerNode::
get_pos() const {
  return _data.get_pos();
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::get_orient
//       Access: Public
//  Description: Returns the current orientation of the tracker, if it
//               is available.
////////////////////////////////////////////////////////////////////
const LOrientationf &TrackerNode::
get_orient() const {
  return _data.get_orient();
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::get_transform
//       Access: Public
//  Description: Returns the current position and orientation of the
//               tracker, as a combined matrix.
////////////////////////////////////////////////////////////////////
const LMatrix4f &TrackerNode::
get_transform() const {
  return _transform;
}

