// Filename: config_util.h
// Created by:  cary (04Jan00)
//
////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_UTIL_H__
#define __CONFIG_UTIL_H__

#include <pandabase.h>
#include <notifyCategoryProxy.h>
#include <dconfig.h>

class DSearchPath;

ConfigureDecl(config_util, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(util, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(bam, EXPCL_PANDA, EXPTP_PANDA);

// Actually, we can't determine this config variable the normal way,
// because we must be able to access it at static init time.  Instead
// of declaring it a global constant, we'll make it a member of
// MemoryUsage.

//extern EXPCL_PANDA const bool track_memory_usage;

// These are functions instead of constant variables because they are
// computed based on the concatenation of all appearances of the
// corresponding variable in the config files.

BEGIN_PUBLISH
EXPCL_PANDA DSearchPath &get_model_path();
EXPCL_PANDA DSearchPath &get_texture_path();
EXPCL_PANDA DSearchPath &get_sound_path();
END_PUBLISH

#endif /* __CONFIG_UTIL_H__ */
