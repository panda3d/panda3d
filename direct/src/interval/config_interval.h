// Filename: config_interval.h
// Created by:  drose (27Aug02)
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

#ifndef CONFIG_INTERVAL_H
#define CONFIG_INTERVAL_H

#include "directbase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

NotifyCategoryDecl(interval, EXPCL_DIRECT, EXPTP_DIRECT);

extern const double interval_precision;
extern const bool verify_intervals;

extern EXPCL_DIRECT void init_libinterval();

#endif
