/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file withOutputFile.I
 * @author drose
 * @date 2003-10-01
 */

/**
 * Changes the flag specifying whether the output file is to be opened in
 * binary mode or not.
 */
INLINE void WithOutputFile::
set_binary_output(bool binary_output) {
  _binary_output = binary_output;
}
