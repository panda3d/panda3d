/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fisheyeLens.I
 * @author drose
 * @date 2001-12-12
 */

/**
 *
 */
INLINE FisheyeLens::
FisheyeLens() {
}

/**
 *
 */
INLINE FisheyeLens::
FisheyeLens(const FisheyeLens &copy) : Lens(copy) {
}

/**
 *
 */
INLINE void FisheyeLens::
operator = (const FisheyeLens &copy) {
  Lens::operator = (copy);
}
