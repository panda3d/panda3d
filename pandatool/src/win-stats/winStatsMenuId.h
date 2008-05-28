// Filename: winStatsMenuId.h
// Created by:  drose (11Jan04)
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
  MI_frame_rate_label,
  MI_speed_1,
  MI_speed_2,
  MI_speed_3,
  MI_speed_6,
  MI_speed_12,
  MI_pause,

  // This one is last and represents the beginning of the range for
  // the various "new chart" menu options.
  MI_new_chart
};

#endif

