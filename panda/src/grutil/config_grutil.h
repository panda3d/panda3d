// Filename: config_grutil.h
// Created by:  drose (24May00)
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

#ifndef CONFIG_GRUTIL_H
#define CONFIG_GRUTIL_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(grutil, EXPCL_PANDA, EXPTP_PANDA);

extern const double frame_rate_meter_update_interval;
extern const string frame_rate_meter_text_pattern;
extern const int frame_rate_meter_layer_sort;
extern const float frame_rate_meter_scale;
extern const float frame_rate_meter_side_margins;

extern EXPCL_PANDA void init_libgrutil();

#endif


