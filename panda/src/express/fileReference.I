/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fileReference.I
 * @author drose
 * @date 2011-06-23
 */

/**
 *
 */
INLINE FileReference::
FileReference(const Filename &filename) : _filename(filename) {
}

/**
 * Returns the filename of the reference.
 */
INLINE const Filename &FileReference::
get_filename() const {
  return _filename;
}
