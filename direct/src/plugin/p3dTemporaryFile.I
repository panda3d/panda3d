/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dTemporaryFile.I
 * @author drose
 * @date 2009-08-19
 */

/**
 * Returns the temporary filename.
 */
inline const std::string &P3DTemporaryFile::
get_filename() const {
  return _filename;
}
