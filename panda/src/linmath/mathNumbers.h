// Filename: mathNumbers.h
// Created by:  mike (23Jan99)
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
#ifndef MATHNUMBERS_H
#define MATHNUMBERS_H

#include "pandabase.h"

class EXPCL_PANDA_LINMATH MathNumbers {
PUBLISHED:
  static const float pi_f;
  static const float rad_2_deg_f;
  static const float deg_2_rad_f;
  static const float ln2_f;

  static const double pi;
  static const double ln2;
  static const double rad_2_deg;
  static const double deg_2_rad;

public:
  INLINE static float cpi(float);
  INLINE static float cln2(float);

  INLINE static double cpi(double);
  INLINE static double cln2(double);
};

#include "mathNumbers.I"

#endif
