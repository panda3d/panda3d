/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file maya_funcs.I
 * @author drose
 * @date 2002-04-15
 */

/**
 *
 */
INLINE std::ostream &operator << (std::ostream &out, const MString &str) {
  return out << str.asChar();
}

/**
 *
 */
INLINE std::ostream &operator << (std::ostream &out, const MVector &vec) {
  return out << vec.x << " " << vec.y << " " << vec.z;
}
