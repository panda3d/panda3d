// Filename: config_skel.h
// Created by:  jyelon (09Feb07)
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

#ifndef CONFIG_SKEL_H
#define CONFIG_SKEL_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableString.h"
#include "configVariableInt.h"

NotifyCategoryDecl(skel, EXPCL_PANDASKEL, EXPTP_PANDASKEL);

extern ConfigVariableInt    skel_sample_config_variable;

extern EXPCL_PANDASKEL void init_libskel();

#endif


