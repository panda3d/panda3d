// Filename: vector_PT_Light.h
// Created by:  drose (16May00)
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

#ifndef VECTOR_PT_LIGHT_H
#define VECTOR_PT_LIGHT_H

#include <pandabase.h>

#include "pt_Light.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_PT_Light
// Description : A vector of PT(Light)'s.  This class is defined once
//               here, and exported to PANDA.DLL; other packages that
//               want to use a vector of this type (whether they need
//               to export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDA
#define EXPTP EXPTP_PANDA
#define TYPE PT_Light
#define NAME vector_PT_Light

#include <vector_src.h>

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
