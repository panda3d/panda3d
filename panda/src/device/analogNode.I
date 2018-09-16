/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file analogNode.I
 * @author drose
 * @date 2002-03-12
 */

/**
 *
 */
INLINE AnalogNode::OutputData::
OutputData() {
  _index = -1;
  _flip = false;
}

/**
 * Returns true if the AnalogNode is valid and connected to a server, false
 * otherwise.
 */
INLINE bool AnalogNode::
is_valid() const {
  return (_analog != nullptr) && _analog->is_connected();
}

/**
 * Returns the number of analog controls known to the AnalogNode.  This number
 * may change as more controls are discovered.
 */
INLINE int AnalogNode::
get_num_controls() const {
  return _analog->get_num_axes();
}

/**
 * Returns the current position of indicated analog control identified by its
 * index number, or 0.0 if the control is unknown.  The normal range of a
 * single control is -1.0 to 1.0.
 */
INLINE double AnalogNode::
get_control_state(int index) const {
  return _analog->get_axis_value(index);
}

/**
 * Returns true if the state of the indicated analog control is known, or
 * false if we have never heard anything about this particular control.
 */
INLINE bool AnalogNode::
is_control_known(int index) const {
  return _analog->is_axis_known(index);
}

/**
 * Causes a particular analog control to be placed in the data graph for the
 * indicated channel.  Normally, a mouse uses channels 0 and 1 for the X and Y
 * information, respectively; channels 0, 1, and 2 are available.  If flip is
 * true, the analog control value will be reversed before outputting it.
 */
INLINE void AnalogNode::
set_output(int channel, int index, bool flip) {
  nassertv(channel >= 0 && channel < max_outputs);
  _outputs[channel]._index = index;
  _outputs[channel]._flip = flip;
}

/**
 * Removes the output to the data graph associated with the indicated channel.
 * See set_output().
 */
INLINE void AnalogNode::
clear_output(int channel) {
  nassertv(channel >= 0 && channel < max_outputs);
  _outputs[channel]._index = -1;
}

/**
 * Returns the analog control index that is output to the data graph on the
 * indicated channel, or -1 if no control is output on that channel.  See
 * set_output().
 */
INLINE int AnalogNode::
get_output(int channel) const {
  nassertr(channel >= 0 && channel < max_outputs, -1);
  return _outputs[channel]._index;
}

/**
 * Returns true if the analog control index that is output to the data graph
 * on the indicated channel is flipped.  See set_output().
 */
INLINE bool AnalogNode::
is_output_flipped(int channel) const {
  nassertr(channel >= 0 && channel < max_outputs, false);
  return _outputs[channel]._flip;
}
