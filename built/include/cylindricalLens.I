/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cylindricalLens.I
 * @author drose
 * @date 2001-12-12
 */

/**
 *
 */
INLINE CylindricalLens::
CylindricalLens() {
}

/**
 *
 */
INLINE CylindricalLens::
CylindricalLens(const CylindricalLens &copy) : Lens(copy) {
}

/**
 *
 */
INLINE void CylindricalLens::
operator = (const CylindricalLens &copy) {
  Lens::operator = (copy);
}
