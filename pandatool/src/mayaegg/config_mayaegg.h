// Filename: config_mayaegg.h
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

#ifndef CONFIG_MAYAEGG_H
#define CONFIG_MAYAEGG_H

#include "pandatoolbase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDeclNoExport(mayaegg);

extern bool maya_default_double_sided;
extern bool maya_default_vertex_color;

extern void init_libmayaegg();

#endif
