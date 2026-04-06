/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramNodePath.I
 * @author rdb
 * @date 2015-02-25
 */

/**
 * Returns NodePath::get_class_type().
 */
INLINE TypeHandle ParamNodePath::
get_value_type() const {
  return NodePath::get_class_type();
}

/**
 * Retrieves the NodePath stored in the parameter.
 */
INLINE NodePath ParamNodePath::
get_value() const {
  return _node_path;
}
