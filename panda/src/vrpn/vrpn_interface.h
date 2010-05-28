// Filename: vrpn_interface.h
// Created by:  drose (25Jan01)
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

#ifndef VRPN_INTERFACE_H
#define VRPN_INTERFACE_H

#include "pandabase.h"

#ifdef CPPPARSER
  // For correct interrogate parsing of UNC's vrpn library.
  #if defined(WIN32_VC) || defined(WIN64_VC)
    #define _WIN32
    #define SOCKET int
  #else
    #define linux
    typedef struct timeval timeval;
  #endif
#endif

#include "vrpn_Connection.h"
#include "vrpn_Tracker.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_Dial.h"

#ifdef sleep
#undef sleep
#endif

#endif
