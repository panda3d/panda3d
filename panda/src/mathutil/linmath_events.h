// Filename: linmath_events.h
// Created by:  drose (12Mar02)
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

#ifndef LINMATH_EVENTS_H
#define LINMATH_EVENTS_H

#include "pandabase.h"

#include "paramValue.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//
// This file defines a few more EventStore classes for storing linmath
// objects in an EventParameter.  These are just for backward
// compatibility; they are defined as typedefs to ParamVecBase types.
//
// See paramValue.h.
//
////////////////////////////////////////////////////////////////////

typedef ParamVecBase2 EventStoreVec2;
typedef ParamVecBase3 EventStoreVec3;
typedef ParamMatrix4 EventStoreMat4;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
