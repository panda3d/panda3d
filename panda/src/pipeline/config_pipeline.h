// Filename: config_pipeline.h
// Created by:  cary (04Jan00)
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

#ifndef CONFIG_PIPELINE_H
#define CONFIG_PIPELINE_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableInt.h"
#include "configVariableBool.h"

ConfigureDecl(config_pipeline, EXPCL_PANDA_PIPELINE, EXPTP_PANDA_PIPELINE);
NotifyCategoryDecl(pipeline, EXPCL_PANDA_PIPELINE, EXPTP_PANDA_PIPELINE);
NotifyCategoryDecl(thread, EXPCL_PANDA_PIPELINE, EXPTP_PANDA_PIPELINE);

extern EXPCL_PANDA_PIPELINE ConfigVariableBool support_threads;
extern ConfigVariableBool name_deleted_mutexes;
extern ConfigVariableInt thread_stack_size;

extern EXPCL_PANDA_PIPELINE void init_libpipeline();

#endif  // CONFIG_PIPELINE_H

