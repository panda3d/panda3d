/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorWriteAttrib.I
 * @author drose
 * @date 2002-03-04
 */

/**
 * Use ColorWriteAttrib::make() to construct a new ColorWriteAttrib object.
 */
INLINE ColorWriteAttrib::
ColorWriteAttrib(unsigned int channels) :
  _channels(channels)
{
}

/**
 * Returns the mask of color channels that are enabled by this attrib.
 */
INLINE unsigned int ColorWriteAttrib::
get_channels() const {
  return _channels;
}
