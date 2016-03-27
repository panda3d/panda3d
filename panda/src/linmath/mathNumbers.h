/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathNumbers.h
 * @author mike
 * @date 1999-01-23
 */

#ifndef MATHNUMBERS_H
#define MATHNUMBERS_H

#include "pandabase.h"
#include "numeric_types.h"

class EXPCL_PANDA_LINMATH MathNumbers {
PUBLISHED:
  static const double pi_d;
  static const double ln2_d;
  static const double rad_2_deg_d;
  static const double deg_2_rad_d;

  static const float pi_f;
  static const float ln2_f;
  static const float rad_2_deg_f;
  static const float deg_2_rad_f;

  static const PN_stdfloat pi;
  static const PN_stdfloat ln2;
  static const PN_stdfloat rad_2_deg;
  static const PN_stdfloat deg_2_rad;

public:
  INLINE static float cpi(float);
  INLINE static float cln2(float);

  INLINE static double cpi(double);
  INLINE static double cln2(double);
};

#include "mathNumbers.I"

#endif
