// Filename: animationConvert.h
// Created by:  drose (21Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
  AC_invalid,
  AC_none,
  AC_flip,
};

string format_animation_convert(AnimationConvert unit);

ostream &operator << (ostream &out, AnimationConvert unit);
AnimationConvert string_animation_convert(const string &str);

#endif
