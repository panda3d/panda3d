/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthWriteAttrib.I
 * @author drose
 * @date 2002-03-04
 */

/**
 * Use DepthWriteAttrib::make() to construct a new DepthWriteAttrib object.
 */
INLINE DepthWriteAttrib::
DepthWriteAttrib(DepthWriteAttrib::Mode mode) :
  _mode(mode)
{
}

/**
 * Returns the depth write mode.
 */
INLINE DepthWriteAttrib::Mode DepthWriteAttrib::
get_mode() const {
  return _mode;
}
