/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file orthographicLens.I
 * @author mike
 * @date 1999-02-18
 */

/**
 *
 */
INLINE OrthographicLens::
OrthographicLens() {
}

/**
 *
 */
INLINE OrthographicLens::
OrthographicLens(const OrthographicLens &copy) : Lens(copy) {
}

/**
 *
 */
INLINE void OrthographicLens::
operator = (const OrthographicLens &copy) {
  Lens::operator = (copy);
}
