// Filename: config_sgraphutil.h
// Created by:  drose (21Feb00)
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

#ifndef CONFIG_SGRAPHUTIL_H
#define CONFIG_SGRAPHUTIL_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

// Configure variables for sgraphutil package.

NotifyCategoryDecl(sgraphutil, EXPCL_PANDA, EXPTP_PANDA);

extern EXPCL_PANDA const bool view_frustum_cull;
extern EXPCL_PANDA const bool fake_view_frustum_cull;
extern EXPCL_PANDA const bool implicit_app_traversal;


#endif
