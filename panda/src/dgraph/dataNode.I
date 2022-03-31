/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dataNode.I
 * @author drose
 * @date 2002-03-11
 */

/**
 *
 */
INLINE DataNode::
DataNode(const std::string &name) :
  PandaNode(name)
{
}

/**
 *
 */
INLINE DataNode::
DataNode(const DataNode &copy) :
  PandaNode(copy)
{
  // Copying a DataNode doesn't copy its inputs or outputs.
}

/**
 * Returns the number of different inputs that have been defined for this node
 * using define_input().  This indicates the size of the DataNodeTransmit
 * object that should be passed to do_transmit_data().
 */
INLINE int DataNode::
get_num_inputs() const {
  return _input_wires.size();
}

/**
 * Returns the number of different outputs that have been defined for this
 * node using define_output().  This indicates the size of the
 * DataNodeTransmit object that should be passed to do_transmit_data().
 */
INLINE int DataNode::
get_num_outputs() const {
  return _output_wires.size();
}
