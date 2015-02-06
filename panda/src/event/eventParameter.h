// Filename: eventParameter.h
// Created by:  drose (08Feb99)
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

#ifndef EVENTPARAMETER_H
#define EVENTPARAMETER_H

#include "pandabase.h"

#include "typedef.h"
#include "parameter.h"

// Some typedefs for backward compatibility.
typedef Parameter EventParameter;

typedef ParamValueBase EventStoreValueBase;
typedef ParamString EventStoreString;
typedef ParamWstring EventStoreWstring;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
