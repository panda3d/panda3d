// Filename: config_egg.h
// Created by:  drose (19Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_EGG_H
#define CONFIG_EGG_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

class DSearchPath;

NotifyCategoryDecl(egg, EXPCL_PANDAEGG, EXPTP_PANDAEGG);

const DSearchPath &get_egg_path();

extern EXPCL_PANDAEGG void init_libegg();

#endif
