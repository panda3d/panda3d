// Filename: config_pgui.h
// Created by:  drose (02Jul01)
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

#ifndef CONFIG_PGUI_H
#define CONFIG_PGUI_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(pgui, EXPCL_PANDA, EXPTP_PANDA);

// Configure variables for pgui package.

extern EXPCL_PANDA void init_libpgui();

#endif
