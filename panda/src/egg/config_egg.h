// Filename: config_egg.h
// Created by:  drose (19Mar00)
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

#ifndef CONFIG_EGG_H
#define CONFIG_EGG_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

class DSearchPath;

NotifyCategoryDecl(egg, EXPCL_PANDAEGG, EXPTP_PANDAEGG);

const DSearchPath &get_egg_path();
extern bool egg_support_old_anims;

extern EXPCL_PANDAEGG void init_libegg();

#endif
