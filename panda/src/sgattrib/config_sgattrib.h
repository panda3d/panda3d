// Filename: config_sgattrib.h
// Created by:  drose (10Mar00)
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

#ifndef CONFIG_SGATTRIB_H
#define CONFIG_SGATTRIB_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(sgattrib, EXPCL_PANDA, EXPTP_PANDA);

// Configure variables for sgattrib package.
enum SupportDirect {
  SD_on,
  SD_off,
  SD_hide
};

extern EXPCL_PANDA SupportDirect support_decals;
extern EXPCL_PANDA SupportDirect support_subrender;
extern EXPCL_PANDA SupportDirect support_direct;

#endif
