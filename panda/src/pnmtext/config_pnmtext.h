// Filename: config_pnmtext.h
// Created by:  drose (08Sep03)
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

#ifndef CONFIG_PNMTEXT_H
#define CONFIG_PNMTEXT_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableBool.h"

class DSearchPath;

NotifyCategoryDecl(pnmtext, EXPCL_PANDA_PNMTEXT, EXPTP_PANDA_PNMTEXT);

extern ConfigVariableDouble text_point_size;
extern ConfigVariableDouble text_pixels_per_unit;
extern ConfigVariableDouble text_scale_factor;
extern ConfigVariableBool text_native_antialias;

extern EXPCL_PANDA_PNMTEXT void init_libpnmtext();

#endif
