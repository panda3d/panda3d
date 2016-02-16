/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file connectionReader.I
 * @author drose
 * @date 2009-02-15
 */

/**
 * Returns true if the reader is a polling reader, i.e.  it has no threads.
 */
INLINE bool ConnectionReader::
is_polling() const {
  return _polling;
}
