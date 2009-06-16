// Filename: config_flt.h
// Created by:  drose (24Aug00)
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

#ifndef CONFIG_FLT_H
#define CONFIG_FLT_H

#include "pandatoolbase.h"

#include "notifyCategoryProxy.h"
#include "configVariableBool.h"

NotifyCategoryDeclNoExport(flt);

extern ConfigVariableBool flt_error_abort;

extern void init_libflt();

static const int header_size = 4;

#endif
