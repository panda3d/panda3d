/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file unionBoundingVolume.I
 * @author drose
 * @date 2012-02-08
 */

/**
 * Constructs an empty union.
 */
INLINE_MATHUTIL UnionBoundingVolume::
UnionBoundingVolume() {
}

/**
 * Returns the number of components in the union.
 */
INLINE_MATHUTIL int UnionBoundingVolume::
get_num_components() const {
  return (int)_components.size();
}

/**
 * Returns the nth component in the union.
 */
INLINE_MATHUTIL const GeometricBoundingVolume *UnionBoundingVolume::
get_component(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), nullptr);
  return _components[n];
}
