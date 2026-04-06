/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file isoPlacer.I
 * @author drose
 * @date 2003-10-13
 */

/**
 *
 */
INLINE IsoPlacer::
IsoPlacer() {
}


/**
 *
 */
INLINE double IsoPlacer::
get_total_score() const {
  return _cint[_maxi];
}
