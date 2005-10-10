// Filename: config_ptloader.h
// Created by:  drose (26Apr01)
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

#ifndef CONFIG_PTLOADER_H
#define CONFIG_PTLOADER_H

#include "pandatoolbase.h"

#include "dconfig.h"
#include "distanceUnit.h"
#include "configVariableEnum.h"

ConfigureDecl(config_ptloader, EXPCL_PTLOADER, EXPTP_PTLOADER);
NotifyCategoryDecl(ptloader, EXPCL_PTLOADER, EXPTP_PTLOADER);

extern ConfigVariableEnum<DistanceUnit> ptloader_units;

extern EXPCL_PTLOADER void init_libptloader();

#endif
