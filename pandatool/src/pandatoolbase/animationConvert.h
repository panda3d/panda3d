/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animationConvert.h
 * @author drose
 * @date 2003-01-21
 */

#ifndef ANIMATIONCONVERT_H
#define ANIMATIONCONVERT_H

#include "pandatoolbase.h"

/**
 * This enumerated type lists the methods by which animation from an animation
 * package might be represented in egg format.
 */
enum AnimationConvert {
  AC_invalid,  // Never use this.
  AC_none,     // No animation: static geometry only.
  AC_pose,     // Pose to one frame, then get static geometry.
  AC_flip,     // A flip (sequence) of static geometry models.
  AC_strobe,   // All frames of a flip visible at the same time.
  AC_model,    // A character model, with joints.
  AC_chan,     // Animation tables for the above model.
  AC_both,     // A character model and tables in the same file.
};

std::string format_animation_convert(AnimationConvert unit);

std::ostream &operator << (std::ostream &out, AnimationConvert unit);
AnimationConvert string_animation_convert(const std::string &str);

#endif
