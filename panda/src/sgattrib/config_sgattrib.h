// Filename: config_sgattrib.h
// Created by:  drose (10Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_SGATTRIB_H
#define CONFIG_SGATTRIB_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(sgattrib, EXPCL_PANDA, EXPTP_PANDA);

// Configure variables for sgattrib package.
enum SupportDecals {
  SD_on,
  SD_off,
  SD_hide,
  SD_invalid
};

extern EXPCL_PANDA SupportDecals support_decals;

#endif
