// Filename: config_cull.h
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_CULL_H
#define CONFIG_CULL_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>
#include <dconfig.h>

ConfigureDecl(config_cull, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(cull, EXPCL_PANDA, EXPTP_PANDA);

extern const bool cull_force_update;

#endif
