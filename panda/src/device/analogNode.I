// Filename: analogNode.I
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
//     Function: AnalogNode::OutputData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE AnalogNode::OutputData::
OutputData() {
  _index = -1;
  _flip = false;
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::is_valid
//       Access: Public
//  Description: Returns true if the AnalogNode is valid and
//               connected to a server, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool AnalogNode::
is_valid() const {
  return (_analog != (ClientAnalogDevice *)NULL) && _analog->is_connected();
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::get_num_controls
//       Access: Public
//  Description: Returns the number of analog controls known to the
//               AnalogNode.  This number may change as more controls
//               are discovered.
////////////////////////////////////////////////////////////////////
INLINE int AnalogNode::
get_num_controls() const {
  _analog->acquire();
  int result = _analog->get_num_controls();
  _analog->unlock();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::get_control_state
//       Access: Public
//  Description: Returns the current position of indicated analog
//               control identified by its index number, or 0.0 if
//               the control is unknown.  The normal range of a single
//               control is -1.0 to 1.0.
////////////////////////////////////////////////////////////////////
INLINE double AnalogNode::
get_control_state(int index) const {
  _analog->acquire();
  double result = _analog->get_control_state(index);
  _analog->unlock();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::is_control_known
//       Access: Public
//  Description: Returns true if the state of the indicated analog
//               control is known, or false if we have never heard
//               anything about this particular control.
////////////////////////////////////////////////////////////////////
INLINE bool AnalogNode::
is_control_known(int index) const {
  _analog->acquire();
  bool result = _analog->is_control_known(index);
  _analog->unlock();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::set_output
//       Access: Public
//  Description: Causes a particular analog control to be placed in
//               the data graph for the indicated channel.  Normally,
//               a mouse uses channels 0 and 1 for the X and Y
//               information, respectively; channels 0, 1, and 2 are
//               available.  If flip is true, the analog control value
//               will be reversed before outputting it.
////////////////////////////////////////////////////////////////////
INLINE void AnalogNode::
set_output(int channel, int index, bool flip) {
  nassertv(channel >= 0 && channel < max_outputs);
  _outputs[channel]._index = index;
  _outputs[channel]._flip = flip;
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::clear_output
//       Access: Public
//  Description: Removes the output to the data graph associated with
//               the indicated channel.  See set_output().
////////////////////////////////////////////////////////////////////
INLINE void AnalogNode::
clear_output(int channel) {
  nassertv(channel >= 0 && channel < max_outputs);
  _outputs[channel]._index = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::get_output
//       Access: Public
//  Description: Returns the analog control index that is output to
//               the data graph on the indicated channel, or -1 if no
//               control is output on that channel.  See set_output().
////////////////////////////////////////////////////////////////////
INLINE int AnalogNode::
get_output(int channel) const {
  nassertr(channel >= 0 && channel < max_outputs, -1);
  return _outputs[channel]._index;
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::is_output_flipped
//       Access: Public
//  Description: Returns true if the analog control index that is
//               output to the data graph on the indicated channel is
//               flipped.  See set_output().
////////////////////////////////////////////////////////////////////
INLINE bool AnalogNode::
is_output_flipped(int channel) const {
  nassertr(channel >= 0 && channel < max_outputs, false);
  return _outputs[channel]._flip;
}
