// Filename: dataNode.I
// Created by:  drose (11Mar02)
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
