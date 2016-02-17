/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cConstrainTransformInterval.I
 * @author pratt
 * @date 2006-09-29
 */

/**
 * Returns the "source" node.
 */
INLINE const NodePath &CConstrainTransformInterval::
get_node() const {
  return _node;
}

/**
 * Returns the "target" node.
 */
INLINE const NodePath &CConstrainTransformInterval::
get_target() const {
  return _target;
}
