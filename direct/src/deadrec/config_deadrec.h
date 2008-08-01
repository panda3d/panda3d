// Filename: config_deadrec.h
// Created by:  drose (23Oct06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_DEADREC_H
#define CONFIG_DEADREC_H

#include "directbase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"

NotifyCategoryDecl(deadrec, EXPCL_DIRECT, EXPTP_DIRECT);

extern ConfigVariableBool accept_clock_skew;

extern EXPCL_DIRECT void init_libdeadrec();

#endif


