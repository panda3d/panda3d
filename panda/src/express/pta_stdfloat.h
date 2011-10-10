// Filename: pta_stdfloat.h
// Created by:  drose (07Oct11)
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

#ifndef PTA_STDFLOAT_H
#define PTA_STDFLOAT_H

#include "pandabase.h"

#include "pta_double.h"
#include "pta_float.h"

#ifndef STDFLOAT_DOUBLE
typedef PTA_float PTA_stdfloat;
typedef CPTA_float CPTA_stdfloat;
#else
typedef PTA_double PTA_stdfloat;
typedef CPTA_double CPTA_stdfloat;
#endif  // STDFLOAT_DOUBLE

#endif
