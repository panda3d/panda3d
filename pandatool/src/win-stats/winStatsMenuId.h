// Filename: winStatsMenuId.h
// Created by:  drose (11Jan04)
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

