// Filename: dataNode.I
// Created by:  drose (11Mar02)
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
//     Function: DataNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DataNode::
DataNode(const string &name) :
  PandaNode(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DataNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DataNode::
DataNode(const DataNode &copy) :
  PandaNode(copy)
{
  // Copying a DataNode doesn't copy its inputs or outputs.
}

////////////////////////////////////////////////////////////////////
//     Function: DataNode::get_num_inputs
//       Access: Public
//  Description: Returns the number of different inputs that have been
//               defined for this node using define_input().  This
//               indicates the size of the DataNodeTransmit object
//               that should be passed to do_transmit_data().
////////////////////////////////////////////////////////////////////
INLINE int DataNode::
get_num_inputs() const {
  return _input_wires.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DataNode::get_num_outputs
//       Access: Public
//  Description: Returns the number of different outputs that have been
//               defined for this node using define_output().  This
//               indicates the size of the DataNodeTransmit object
//               that should be passed to do_transmit_data().
////////////////////////////////////////////////////////////////////
INLINE int DataNode::
get_num_outputs() const {
  return _output_wires.size();
}
