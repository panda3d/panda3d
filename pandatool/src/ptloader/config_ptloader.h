// Filename: config_ptloader.h
// Created by:  drose (26Apr01)
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

#ifndef CONFIG_PTLOADER_H
#define CONFIG_PTLOADER_H

#include "pandatoolbase.h"

#include "dconfig.h"
#include "distanceUnit.h"
#include "configVariableEnum.h"
#include "configVariableBool.h"

ConfigureDecl(config_ptloader, EXPCL_PTLOADER, EXPTP_PTLOADER);
NotifyCategoryDecl(ptloader, EXPCL_PTLOADER, EXPTP_PTLOADER);

extern ConfigVariableEnum<DistanceUnit> ptloader_units;
extern ConfigVariableBool ptloader_load_node;

extern EXPCL_PTLOADER void init_libptloader();

#endif
