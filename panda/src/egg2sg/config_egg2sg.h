// Filename: config_egg2sg.h
// Created by:  drose (01Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_EGG2SG_H
#define CONFIG_EGG2SG_H

#include <pandabase.h>

#include <coordinateSystem.h>
#include <typedef.h>
#include <notifyCategoryProxy.h>
#include <dconfig.h>

#include "config_egg2pg.h" // temp to declare the global consts

ConfigureDecl(config_egg2sg, EXPCL_PANDAEGG, EXPTP_PANDAEGG);
NotifyCategoryDecl(egg2sg, EXPCL_PANDAEGG, EXPTP_PANDAEGG);

extern EXPCL_PANDAEGG void init_libegg2sg();

#endif
