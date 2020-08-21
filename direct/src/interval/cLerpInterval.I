/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLerpInterval.I
 * @author drose
 * @date 2002-08-27
 */

/**
 *
 */
INLINE CLerpInterval::
CLerpInterval(const std::string &name, double duration,
              CLerpInterval::BlendType blend_type) :
  CInterval(name, duration, true),
  _blend_type(blend_type)
{
}

/**
 * Returns the blend type specified for the interval.  This controls how the
 * linear interpolation behaves near the beginning and end of the lerp period.
 */
INLINE CLerpInterval::BlendType CLerpInterval::
get_blend_type() const {
  return _blend_type;
}
