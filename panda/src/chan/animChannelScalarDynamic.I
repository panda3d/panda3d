/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelScalarDynamic.I
 * @author drose
 * @date 2003-10-20
 */

/**
 * Gets the value of the channel.  This will return the value explicitly
 * specified by set_value() unless a value node was specified using
 * set_value_node().
 */
INLINE PN_stdfloat AnimChannelScalarDynamic::
get_value() const {
  if (_value_node != nullptr) {
    return _value->get_pos()[0];
  } else {
    return _float_value;
  }
}

/**
 * Returns the node that was set via set_value_node(), if any.
 */
INLINE PandaNode *AnimChannelScalarDynamic::
get_value_node() const {
  return _value_node;
}
