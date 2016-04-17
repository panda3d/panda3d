/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file androidGraphicsStateGuardian.I
 * @author rdb
 * @date 2013-01-11
 */

/**
 * Gets the FrameBufferProperties for all windows and buffers that use this
 * GSG.
 */
INLINE const FrameBufferProperties &AndroidGraphicsStateGuardian::
get_fb_properties() const {
  return _fbprops;
}
