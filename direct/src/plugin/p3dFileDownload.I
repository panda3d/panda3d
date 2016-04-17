/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dFileDownload.I
 * @author drose
 * @date 2009-06-11
 */

/**
 * Returns the filename that we are downloading into.
 */
const string &P3DFileDownload::
get_filename() const {
  return _filename;
}
