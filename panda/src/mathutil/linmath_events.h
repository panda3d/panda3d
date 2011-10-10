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

#include "eventParameter.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//
// This file defines a few more EventStore classes for storing linmath
// objects in an EventParameter.  We can't define this with the other
// EventStore classes, because linmath hasn't been defined yet at that
// point.
//
// See eventParameter.h.
//
////////////////////////////////////////////////////////////////////


EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, EventStoreValue<LVecBase2>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, EventStoreValue<LVecBase3>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, EventStoreValue<LMatrix4>);

typedef EventStoreValue<LVecBase2> EventStoreVec2;
typedef EventStoreValue<LVecBase3> EventStoreVec3;
typedef EventStoreValue<LMatrix4> EventStoreMat4;


// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
