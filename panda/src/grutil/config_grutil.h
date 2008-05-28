// Filename: config_grutil.h
// Created by:  drose (24May00)
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

#ifndef CONFIG_GRUTIL_H
#define CONFIG_GRUTIL_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableString.h"
#include "configVariableInt.h"
#include "configVariableBool.h"

NotifyCategoryDecl(grutil, EXPCL_PANDA_GRUTIL, EXPTP_PANDA_GRUTIL);

extern ConfigVariableDouble frame_rate_meter_update_interval;
extern ConfigVariableString frame_rate_meter_text_pattern;
extern ConfigVariableInt frame_rate_meter_layer_sort;
extern ConfigVariableDouble frame_rate_meter_scale;
extern ConfigVariableDouble frame_rate_meter_side_margins;

extern EXPCL_PANDA_GRUTIL void init_libgrutil();

#endif


