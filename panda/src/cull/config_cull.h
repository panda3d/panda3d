// Filename: config_cull.h
// Created by:  drose (23Mar06)
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

#ifndef CONFIG_CULL_H
#define CONFIG_CULL_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

class DSearchPath;

ConfigureDecl(config_cull, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(cull, EXPCL_PANDA, EXPTP_PANDA);

extern EXPCL_PANDA void init_libcull();

#endif
