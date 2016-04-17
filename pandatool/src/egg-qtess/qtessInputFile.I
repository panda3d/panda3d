/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qtessInputFile.I
 * @author drose
 * @date 2003-10-13
 */

/**
 *
 */
INLINE QtessInputFile::
QtessInputFile(const QtessInputFile &copy) :
  _entries(copy._entries)
{
}

/**
 *
 */
INLINE void QtessInputFile::
operator = (const QtessInputFile &copy) {
  _entries = copy._entries;
}
