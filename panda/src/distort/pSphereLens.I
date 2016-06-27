/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pSphereLens.I
 * @author drose
 * @date 2001-12-12
 */

/**
 *
 */
INLINE PSphereLens::
PSphereLens() {
}

/**
 *
 */
INLINE PSphereLens::
PSphereLens(const PSphereLens &copy) : Lens(copy) {
}

/**
 *
 */
INLINE void PSphereLens::
operator = (const PSphereLens &copy) {
  Lens::operator = (copy);
}
