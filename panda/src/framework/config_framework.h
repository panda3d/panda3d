// Filename: config_framework.h
// Created by:  drose (06Sep00)
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

extern ConfigVariableDouble win_background_r;
extern ConfigVariableDouble win_background_g;
extern ConfigVariableDouble win_background_b;

extern ConfigVariableString record_session;
extern ConfigVariableString playback_session;

#endif
