/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compassEffect.I
 * @author drose
 * @date 2002-07-16
 */

/**
 * Use CompassEffect::make() to construct a new CompassEffect object.
 */
INLINE CompassEffect::
CompassEffect() {
  _properties = 0;
}

/**
 * Returns the reference node from which the CompassEffect inherits its
 * transform.  If this is empty, it means the root of the scene graph.
 */
INLINE const NodePath &CompassEffect::
get_reference() const {
  return _reference;
}

/**
 * Returns the bitmask of properties that this CompassEffect object inherits
 * from its reference node (or from the root).
 */
INLINE int CompassEffect::
get_properties() const {
  return _properties;
}
