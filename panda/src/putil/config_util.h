// Filename: config_util.h
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

#ifndef __CONFIG_UTIL_H__
#define __CONFIG_UTIL_H__

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableSearchPath.h"
#include "configVariableEnum.h"
#include "configVariableDouble.h"
#include "bamEndian.h"
#include "bamTextureMode.h"
#include "dconfig.h"

class DSearchPath;

ConfigureDecl(config_util, EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL);
NotifyCategoryDecl(util, EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL);
NotifyCategoryDecl(bam, EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL);

// Actually, we can't determine this config variable the normal way,
// because we must be able to access it at static init time.  Instead
// of declaring it a global constant, we'll make it a member of
// MemoryUsage.
//extern EXPCL_PANDA_PUTIL const bool track_memory_usage;

extern EXPCL_PANDA_PUTIL ConfigVariableEnum<BamEndian> bam_endian;
extern EXPCL_PANDA_PUTIL ConfigVariableEnum<BamTextureMode> bam_texture_mode;

extern EXPCL_PANDA_PUTIL ConfigVariableSearchPath model_path;
extern EXPCL_PANDA_PUTIL ConfigVariableSearchPath texture_path;
extern EXPCL_PANDA_PUTIL ConfigVariableSearchPath sound_path;
extern EXPCL_PANDA_PUTIL ConfigVariableSearchPath plugin_path;

// The above variables are also shadowed by these functions, so that
// they can easily be accessed in the interpreter (e.g. Python).
BEGIN_PUBLISH
EXPCL_PANDA_PUTIL ConfigVariableSearchPath &get_model_path();
EXPCL_PANDA_PUTIL ConfigVariableSearchPath &get_texture_path();
EXPCL_PANDA_PUTIL ConfigVariableSearchPath &get_sound_path();
EXPCL_PANDA_PUTIL ConfigVariableSearchPath &get_plugin_path();
END_PUBLISH

extern ConfigVariableDouble clock_frame_rate;
extern ConfigVariableDouble clock_degrade_factor;
extern ConfigVariableDouble max_dt;
extern ConfigVariableDouble sleep_precision;
extern ConfigVariableDouble average_frame_rate_interval;

extern EXPCL_PANDA_PUTIL void init_libputil();

#endif /* __CONFIG_UTIL_H__ */
