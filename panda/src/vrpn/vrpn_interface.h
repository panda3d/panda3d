// Filename: vrpn_interface.h
// Created by:  drose (25Jan01)
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

#ifndef VRPN_INTERFACE_H
#define VRPN_INTERFACE_H

#include "pandabase.h"

#ifdef CPPPARSER
  // For correct interrogate parsing of UNC's vrpn library.
  #ifdef WIN32_VC
    #define _WIN32
    #define SOCKET int
  #else
    #define linux
    typedef struct timeval timeval;
  #endif
#endif

#include <vrpn_Connection.h>
#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>
#include <vrpn_Button.h>
#include <vrpn_Dial.h>

#ifdef sleep
#undef sleep
#endif

#endif
