/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file selectiveChildNode.I
 * @author drose
 * @date 2002-03-06
 */

/**
 *
 */
INLINE SelectiveChildNode::
SelectiveChildNode(const std::string &name) :
  PandaNode(name),
  _selected_child(0)
{
}

/**
 *
 */
INLINE SelectiveChildNode::
SelectiveChildNode(const SelectiveChildNode &copy) :
  PandaNode(copy),
  _selected_child(copy._selected_child)
{
}

/**
 * Selects the indicated child for rendering.  This is normally called during
 * the cull_callback() method, but it may be called at any time.
 */
INLINE void SelectiveChildNode::
select_child(int n) {
  nassertv(n >= 0);
  _selected_child = n;
}
