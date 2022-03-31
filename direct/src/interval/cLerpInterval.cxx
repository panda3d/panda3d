/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLerpInterval.cxx
 * @author drose
 * @date 2002-08-27
 */

#include "cLerpInterval.h"
#include "string_utils.h"

TypeHandle CLerpInterval::_type_handle;

/**
 * Returns the BlendType enumerated value corresponding to the indicated
 * string, or BT_invalid if the string doesn't match anything.
 */
CLerpInterval::BlendType CLerpInterval::
string_blend_type(const std::string &blend_type) {
  if (blend_type == "easeIn") {
    return BT_ease_in;
  } else if (blend_type == "easeOut") {
    return BT_ease_out;
  } else if (blend_type == "easeInOut") {
    return BT_ease_in_out;
  } else if (blend_type == "noBlend") {
    return BT_no_blend;
  } else {
    return BT_invalid;
  }
}

/**
 * Given a t value in the range [0, get_duration()], returns the corresponding
 * delta value clamped to the range [0, 1], after scaling by duration and
 * applying the blend type.
 */
double CLerpInterval::
compute_delta(double t) const {
  double duration = get_duration();
  if (duration == 0.0) {
    // If duration is 0, the lerp works as a set.  Thus, the delta is always
    // 1.0, the terminating value.
    return 1.0;
  }
  t /= duration;
  t = std::min(std::max(t, 0.0), 1.0);

  switch (_blend_type) {
  case BT_ease_in:
    {
      double t2 = t * t;
      return ((3.0 * t2) - (t2 * t)) * 0.5;
    }

  case BT_ease_out:
    {
      double t2 = t * t;
      return ((3.0 * t) - (t2 * t)) * 0.5;
    }

  case BT_ease_in_out:
    {
      double t2 = t * t;
      return (3.0 * t2) - (2.0 * t * t2);
    }

  default:
    return t;
  }
}
