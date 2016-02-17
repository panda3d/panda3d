/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoHeader.I
 * @author drose
 * @date 2001-04-24
 */

/**
 * Returns true if the header represents a valid and recognized Lightwave
 * header, false otherwise.
 */
INLINE bool LwoHeader::
is_valid() const {
  return _valid;
}

/**
 * Returns the version of the Lightwave file.
 */
INLINE double LwoHeader::
get_version() const {
  return _version;
}
