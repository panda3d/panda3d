// Filename: config_framework.h
// Created by:  drose (06Sep00)
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

#ifndef CONFIG_FRAMEWORK_H
#define CONFIG_FRAMEWORK_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>
#include <collideMask.h>

NotifyCategoryDecl(framework, EXPCL_EMPTY, EXPCL_EMPTY);

// Configure variables for framework package.
extern const double drive_height;
extern const CollideMask drive_mask;

#endif
