// Filename: winStatsMenuId.h
// Created by:  drose (11Jan04)
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

#ifndef WINSTATSMENUID_H
#define WINSTATSMENUID_H

#include "pandatoolbase.h"

////////////////////////////////////////////////////////////////////
//        Enum : WinStatsMenuId
// Description : The enumerated values here are used for menu ID's for
//               the various pulldown menus in the application.
////////////////////////////////////////////////////////////////////
enum WinStatsMenuId {
  MI_none,
  MI_time_ms,
  MI_time_hz,

  // This one is last and represents the beginning of the range for
  // the various "new chart" menu options.
  MI_new_chart
};

#endif

