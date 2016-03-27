/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoInputFile.I
 * @author drose
 * @date 2001-04-24
 */

/**
 * Returns the version of the Lightwave file being read.  This is unknown
 * until the header record has been read; then it will be set by the header.
 */
INLINE double LwoInputFile::
get_lwo_version() const {
  return _lwo_version;
}


/**
 * Changes the version number reported for the Lightwave file.  Normally this
 * is only called by LwoHeader as it is read.
 */
INLINE void LwoInputFile::
set_lwo_version(double lwo_version) {
  _lwo_version = lwo_version;
}
