// Filename: linmath_events.h
// Created by:  drose (12Mar02)
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


EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, EventStoreValue<LVecBase2f>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, EventStoreValue<LVecBase3f>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, EventStoreValue<LMatrix4f>);

typedef EventStoreValue<LVecBase2f> EventStoreVec2;
typedef EventStoreValue<LVecBase3f> EventStoreVec3;
typedef EventStoreValue<LMatrix4f> EventStoreMat4;


// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
