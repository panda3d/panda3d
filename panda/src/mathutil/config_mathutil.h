// Filename: config_mathutil.h
// Created by:  drose (01Oct99)
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

#ifndef CONFIG_MATHUTIL_H
#define CONFIG_MATHUTIL_H

#include "pandabase.h"
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(mathutil, EXPCL_PANDA, EXPTP_PANDA);

extern const double fft_offset;
extern const double fft_factor;
extern const double fft_exponent;

#endif


