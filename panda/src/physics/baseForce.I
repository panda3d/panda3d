/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseForce.I
 * @author charles
 * @date 2000-08-08
 */

/**

 */
INLINE ForceNode *BaseForce::
get_force_node() const {
  return _force_node;
}

/**

 */
INLINE NodePath BaseForce::
get_force_node_path() const {
  return _force_node_path;
}

/**

 */
INLINE void BaseForce::
set_active(bool active) {
  _active = active;
}

/**

 */
INLINE bool BaseForce::
get_active() const {
  return _active;
}
