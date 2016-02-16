/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthTestAttrib.I
 * @author drose
 * @date 2002-03-04
 */

/**
 * Use DepthTestAttrib::make() to construct a new DepthTestAttrib object.
 */
INLINE DepthTestAttrib::
DepthTestAttrib(DepthTestAttrib::PandaCompareFunc mode) :
  _mode(mode)
{
}

/**
 * Returns the depth write mode.
 */
INLINE DepthTestAttrib::PandaCompareFunc DepthTestAttrib::
get_mode() const {
  return _mode;
}
