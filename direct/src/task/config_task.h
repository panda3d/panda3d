// Filename: config_task.h
// Created by:  Shochet (03Sep04)
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

#ifndef CONFIG_TASK_H
#define CONFIG_TASK_H

#include "directbase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

NotifyCategoryDecl(task, EXPCL_DIRECT, EXPTP_DIRECT);

extern EXPCL_DIRECT void init_libtask();

#endif
