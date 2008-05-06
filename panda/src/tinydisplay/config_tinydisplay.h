// Filename: config_tinydisplay.h
// Created by:  drose (24Apr08)
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

#ifndef CONFIG_TINYDISPLAY_H
#define CONFIG_TINYDISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

NotifyCategoryDecl(tinydisplay, EXPCL_TINYDISPLAY, EXPTP_TINYDISPLAY);

extern EXPCL_TINYDISPLAY void init_libtinydisplay();

extern ConfigVariableString display_cfg;
extern ConfigVariableBool x_error_abort;
extern ConfigVariableInt x_wheel_up_button;
extern ConfigVariableInt x_wheel_down_button;

extern ConfigVariableInt td_texture_ram;

#endif
