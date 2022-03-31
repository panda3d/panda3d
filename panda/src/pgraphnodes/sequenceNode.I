/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sequenceNode.I
 * @author drose
 * @date 2002-03-06
 */

/**
 *
 */
INLINE SequenceNode::
SequenceNode(const std::string &name) :
  SelectiveChildNode(name)
{
  set_cull_callback();
}

/**
 * Changes the advertised frame rate of the SequenceNode.  This can be used in
 * conjunction with get_play_rate() to change the effective frame rate of the
 * node.
 */
INLINE void SequenceNode::
set_frame_rate(double frame_rate) {
  AnimInterface::set_frame_rate(frame_rate);
}
