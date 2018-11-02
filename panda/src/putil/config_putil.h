/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_putil.h
 * @author cary
 * @date 2000-01-04
 */

#ifndef __CONFIG_UTIL_H__
#define __CONFIG_UTIL_H__

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableSearchPath.h"
#include "configVariableEnum.h"
#include "configVariableDouble.h"
#include "configVariableInt.h"
#include "bamEnums.h"
#include "dconfig.h"

class DSearchPath;

ConfigureDecl(config_putil, EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL);
NotifyCategoryDecl(util, EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL);
NotifyCategoryDecl(bam, EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL);

// Actually, we can't determine this config variable the normal way, because
// we must be able to access it at static init time.  Instead of declaring it
// a global constant, we'll make it a member of MemoryUsage.  extern
// EXPCL_PANDA_PUTIL const bool track_memory_usage;

extern EXPCL_PANDA_PUTIL ConfigVariableInt bam_version;
extern EXPCL_PANDA_PUTIL ConfigVariableEnum<BamEnums::BamEndian> bam_endian;
extern EXPCL_PANDA_PUTIL ConfigVariableBool bam_stdfloat_double;
extern EXPCL_PANDA_PUTIL ConfigVariableEnum<BamEnums::BamTextureMode> bam_texture_mode;

BEGIN_PUBLISH
EXPCL_PANDA_PUTIL ConfigVariableSearchPath &get_model_path();
EXPCL_PANDA_PUTIL ConfigVariableSearchPath &get_plugin_path();
END_PUBLISH

extern ConfigVariableDouble sleep_precision;

extern EXPCL_PANDA_PUTIL ConfigVariableBool preload_textures;
extern EXPCL_PANDA_PUTIL ConfigVariableBool preload_simple_textures;
extern EXPCL_PANDA_PUTIL ConfigVariableBool compressed_textures;
extern EXPCL_PANDA_PUTIL ConfigVariableBool cache_check_timestamps;

extern EXPCL_PANDA_PUTIL void init_libputil();

#endif /* __CONFIG_UTIL_H__ */
