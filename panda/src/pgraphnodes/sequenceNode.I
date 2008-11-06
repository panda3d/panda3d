// Filename: sequenceNode.I
// Created by:  drose (06Mar02)
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
//     Function: SequenceNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SequenceNode::
SequenceNode(const string &name) :
  SelectiveChildNode(name)
{
  set_cull_callback();
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::set_frame_rate
//       Access: Published
//  Description: Changes the advertised frame rate of the
//               SequenceNode.  This can be used in conjunction with
//               get_play_rate() to change the effective frame rate of
//               the node.
////////////////////////////////////////////////////////////////////
INLINE void SequenceNode::
set_frame_rate(double frame_rate) {
  AnimInterface::set_frame_rate(frame_rate);
}
