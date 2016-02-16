/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file is_pathsep.I
 * @author drose
 * @date 2009-07-07
 */

/**
 * Returns true if the indicated character is a path separator character (e.g.
 * slash or backslash), false otherwise.
 */
inline bool
is_pathsep(int ch) {
  if (ch == '/') {
    return true;
  }
#ifdef _WIN32
  if (ch == '\\') {
    return true;
  }
#endif
  return false;
}
