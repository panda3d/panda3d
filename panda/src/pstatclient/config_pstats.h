// Filename: config_pstats.h
// Created by:  drose (09Jul00)
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

#ifndef CONFIG_PSTATS_H
#define CONFIG_PSTATS_H

#include "pandabase.h"

#include "notifyCategoryProxy.h"
#include "dconfig.h"

// Configure variables for pstats package.

ConfigureDecl(config_pstats, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pstats, EXPCL_PANDA, EXPTP_PANDA);

extern EXPCL_PANDA string get_pstats_name();
extern EXPCL_PANDA float get_pstats_max_rate();
extern EXPCL_PANDA const string pstats_host;
extern EXPCL_PANDA const int pstats_port;
extern EXPCL_PANDA const float pstats_target_frame_rate;

extern EXPCL_PANDA const bool pstats_scroll_mode;
extern EXPCL_PANDA const float pstats_history;
extern EXPCL_PANDA const float pstats_average_time;

extern EXPCL_PANDA const bool pstats_threaded_write;

extern EXPCL_PANDA void init_libpstatclient();

#endif
