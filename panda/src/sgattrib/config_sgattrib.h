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
enum SupportDirect {
  SD_on,
  SD_off,
  SD_hide
};

extern EXPCL_PANDA SupportDirect support_decals;
extern EXPCL_PANDA SupportDirect support_subrender;
extern EXPCL_PANDA SupportDirect support_direct;

#endif
