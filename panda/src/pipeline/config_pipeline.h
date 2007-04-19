// Filename: config_pipeline.h
// Created by:  cary (04Jan00)
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

#ifndef CONFIG_PIPELINE_H
#define CONFIG_PIPELINE_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableInt.h"
#include "configVariableBool.h"

ConfigureDecl(config_pipeline, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pipeline, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(thread, EXPCL_PANDA, EXPTP_PANDA);

extern ConfigVariableBool support_threads;
extern ConfigVariableInt thread_stack_size;

extern EXPCL_PANDA void init_libpipeline();

#endif  // CONFIG_PIPELINE_H

