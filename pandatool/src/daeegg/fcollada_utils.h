/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fcollada_utils.h
 * @author rdb
 * @date 2008-12-22
 */

// This file defines some conversion tools for conversion between FCollada and
// Panda3D

#ifndef FCOLLADA_UTILS_H
#define FCOLLADA_UTILS_H

#include "pre_fcollada_include.h"
#include <FCollada.h>

// Useful conversion stuff
inline LVecBase3d TO_VEC3(FMVector3 v) {
  return LVecBase3d(v.x, v.y, v.z);
}
inline LVecBase4d TO_VEC4(FMVector4 v) {
  return LVecBase4d(v.x, v.y, v.z, v.w);
}
inline LColor TO_COLOR(FMVector4 v) {
  return LColor(v.x, v.y, v.z, v.w);
}
#define FROM_VEC3(v) (FMVector3(v[0], v[1], v[2]))
#define FROM_VEC4(v) (FMVector4(v[0], v[1], v[2], v[3]))
#define FROM_MAT4(v) (FMMatrix44(v.getData()))
#define FROM_FSTRING(fs) (std::string(fs.c_str()))

#endif
