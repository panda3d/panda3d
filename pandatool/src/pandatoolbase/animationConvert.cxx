/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animationConvert.cxx
 * @author drose
 * @date 2003-01-21
 */

#include "animationConvert.h"

#include "string_utils.h"
#include "pnotify.h"

/**
 * Returns the string corresponding to this method.
 */
std::string
format_animation_convert(AnimationConvert convert) {
  switch (convert) {
  case AC_invalid:
    return "invalid";

  case AC_none:
    return "none";

  case AC_pose:
    return "pose";

  case AC_flip:
    return "flip";

  case AC_strobe:
    return "strobe";

  case AC_model:
    return "model";

  case AC_chan:
    return "chan";

  case AC_both:
    return "both";
  }
  nout << "**unexpected AnimationConvert value: (" << (int)convert << ")**";
  return "**";
}

/**
 *
 */
std::ostream &
operator << (std::ostream &out, AnimationConvert convert) {
  return out << format_animation_convert(convert);
}

/**
 * Converts from a string, as might be input by the user, to one of the known
 * AnimationConvert types.  Returns AC_invalid if the string is unknown.
 */
AnimationConvert
string_animation_convert(const std::string &str) {
  if (cmp_nocase(str, "none") == 0) {
    return AC_none;

  } else if (cmp_nocase(str, "pose") == 0) {
    return AC_pose;

  } else if (cmp_nocase(str, "flip") == 0) {
    return AC_flip;

  } else if (cmp_nocase(str, "strobe") == 0) {
    return AC_strobe;

  } else if (cmp_nocase(str, "model") == 0) {
    return AC_model;

  } else if (cmp_nocase(str, "chan") == 0) {
    return AC_chan;

  } else if (cmp_nocase(str, "both") == 0) {
    return AC_both;

  } else {
    return AC_invalid;
  }
}
