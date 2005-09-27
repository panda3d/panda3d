// Filename: sequenceNode.I
// Created by:  drose (06Mar02)
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
//     Function: SequenceNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SequenceNode::
SequenceNode(const string &name) :
  SelectiveChildNode(name)
{
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
