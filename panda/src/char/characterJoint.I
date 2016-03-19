/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJoint.I
 * @author drose
 * @date 2008-11-20
 */

/**
 * Returns the transform matrix of the joint
 */
INLINE const LMatrix4 &CharacterJoint::
get_transform() const {
  return _value;
}
