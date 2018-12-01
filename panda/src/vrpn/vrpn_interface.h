/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpn_interface.h
 * @author drose
 * @date 2001-01-25
 */

#ifndef VRPN_INTERFACE_H
#define VRPN_INTERFACE_H

#include "pandabase.h"

// VPRN misses an include to this in vrpn_Shared.h.
#include <stdint.h>

// Prevent VRPN from defining this function, which we don't need,
// and cause compilation errors in MSVC 2015.
#include <vrpn_Configure.h>
#undef VRPN_EXPORT_GETTIMEOFDAY

#include <vrpn_Connection.h>
#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>
#include <vrpn_Button.h>
#include <vrpn_Dial.h>

#ifdef sleep
#undef sleep
#endif

#endif
