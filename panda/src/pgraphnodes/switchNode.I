/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file switchNode.I
 * @author drose
 * @date 2002-07-31
 */

/**
 *
 */
INLINE SwitchNode::CData::
CData() {
  _visible_child = 0;
}

/**
 *
 */
INLINE SwitchNode::CData::
CData(const SwitchNode::CData &copy) :
  _visible_child(copy._visible_child)
{
}

/**
 *
 */
INLINE SwitchNode::
SwitchNode(const std::string &name) :
  SelectiveChildNode(name)
{
  set_cull_callback();
}

/**
 * Specifies the particular child of this node, by index, that will be
 * visible.
 */
INLINE void SwitchNode::
set_visible_child(int index) {
  nassertv(index >= 0);
  CDWriter cdata(_cycler);
  cdata->_visible_child = index;
}
