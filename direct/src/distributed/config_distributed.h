// Filename: config_distributed.h
// Created by:  drose (19May04)
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

#ifndef CONFIG_DISTRIBUTED_H
#define CONFIG_DISTRIBUTED_H

#include "directbase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

NotifyCategoryDecl(distributed, EXPCL_DIRECT, EXPTP_DIRECT);

extern const int game_server_timeout_ms;
extern const double min_lag;
extern const double max_lag;

extern EXPCL_DIRECT void init_libdistributed();

#endif

