// Filename: mathNumbers.h
// Created by:  mike (23Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////
#ifndef MATHNUMBERS_H
#define MATHNUMBERS_H

#include "pandabase.h"

class EXPCL_PANDA MathNumbers {
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
