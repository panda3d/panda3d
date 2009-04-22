// Filename: config_maya.h
// Created by:  drose (15Apr02)
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

#ifndef CONFIG_MAYA_H
#define CONFIG_MAYA_H

#include "pandatoolbase.h"
#include "notifyCategoryProxy.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"

NotifyCategoryDeclNoExport(maya);

extern ConfigVariableInt init_maya_repeat_count;
extern ConfigVariableDouble init_maya_timeout;

extern void init_libmaya();

#endif
