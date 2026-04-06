/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file oSphereLens.I
 * @author drose
 * @date 2011-02-25
 */

/**
 *
 */
INLINE OSphereLens::
OSphereLens() {
}

/**
 *
 */
INLINE OSphereLens::
OSphereLens(const OSphereLens &copy) : Lens(copy) {
}

/**
 *
 */
INLINE void OSphereLens::
operator = (const OSphereLens &copy) {
  Lens::operator = (copy);
}
