/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelMatrixDynamic.I
 * @author drose
 * @date 2003-10-20
 */

/**
 * Returns the explicit TransformState value that was set via set_value(), if
 * any.
 */
INLINE const TransformState *AnimChannelMatrixDynamic::
get_value_transform() const {
  return _value;
}

/**
 * Returns the node that was set via set_value_node(), if any.
 */
INLINE PandaNode *AnimChannelMatrixDynamic::
get_value_node() const {
  return _value_node;
}
