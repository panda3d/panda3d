// Filename: config_softegg.h
// Created by:  masad (25Sep03)
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

#ifndef CONFIG_SOFTEGG_H
#define CONFIG_SOFTEGG_H

#include "pandatoolbase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"

NotifyCategoryDeclNoExport(softegg);

extern ConfigVariableBool soft_default_double_sided;
extern ConfigVariableBool soft_default_vertex_color;

extern void init_libsoftegg();

#endif
