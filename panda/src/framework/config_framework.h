// Filename: config_framework.h
// Created by:  drose (06Sep00)
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

#ifndef CONFIG_FRAMEWORK_H
#define CONFIG_FRAMEWORK_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "windowProperties.h"
#include "configVariableDouble.h"
#include "configVariableBool.h"
#include "configVariableString.h"

NotifyCategoryDecl(framework, EXPCL_FRAMEWORK, EXPTP_FRAMEWORK);

// Configure variables for framework package.
extern ConfigVariableDouble aspect_ratio;
extern ConfigVariableBool show_frame_rate_meter;
extern ConfigVariableBool show_scene_graph_analyzer_meter;
extern ConfigVariableString window_type;

extern ConfigVariableString record_session;
extern ConfigVariableString playback_session;

#endif
