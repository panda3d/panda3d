// Filename: animationConvert.h
// Created by:  drose (21Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef ANIMATIONCONVERT_H
#define ANIMATIONCONVERT_H

#include "pandatoolbase.h"

////////////////////////////////////////////////////////////////////
//        Enum : AnimationConvert
// Description : This enumerated type lists the methods by which
//               animation from an animation package might be
//               represented in egg format.
////////////////////////////////////////////////////////////////////
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

string format_animation_convert(AnimationConvert unit);

ostream &operator << (ostream &out, AnimationConvert unit);
AnimationConvert string_animation_convert(const string &str);

#endif
