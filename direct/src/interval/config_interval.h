// Filename: config_interval.h
// Created by:  drose (27Aug02)
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

#ifndef CONFIG_INTERVAL_H
#define CONFIG_INTERVAL_H

#include "directbase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableDouble.h"
#include "configVariableBool.h"

NotifyCategoryDecl(interval, EXPCL_DIRECT, EXPTP_DIRECT);

extern ConfigVariableDouble interval_precision;
extern EXPCL_DIRECT ConfigVariableBool verify_intervals;

extern EXPCL_DIRECT void init_libinterval();

#endif
