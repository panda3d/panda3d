// Filename: config_grutil.h
// Created by:  drose (24May00)
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

#ifndef CONFIG_GRUTIL_H
#define CONFIG_GRUTIL_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableString.h"
#include "configVariableInt.h"

NotifyCategoryDecl(grutil, EXPCL_PANDA, EXPTP_PANDA);

extern ConfigVariableDouble frame_rate_meter_update_interval;
extern ConfigVariableString frame_rate_meter_text_pattern;
extern ConfigVariableInt frame_rate_meter_layer_sort;
extern ConfigVariableDouble frame_rate_meter_scale;
extern ConfigVariableDouble frame_rate_meter_side_margins;

extern EXPCL_PANDA void init_libgrutil();

#endif


