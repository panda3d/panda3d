// Filename: config_loader.h
// Created by:  drose (19Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <pandabase.h>

#include <dconfig.h>
#include <notifyCategoryProxy.h>

class DSearchPath;

NotifyCategoryDecl(loader, EXPCL_PANDA, EXPTP_PANDA);

extern const bool asynchronous_loads;
const DSearchPath &get_bam_path();

extern Config::ConfigTable::Symbol *load_file_type;

#endif
