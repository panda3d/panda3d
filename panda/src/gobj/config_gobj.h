// Filename: config_gobj.h
// Created by:  drose (01Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_GOBJ_H
#define CONFIG_GOBJ_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(gobj, EXPCL_PANDA, EXPTP_PANDA);

// Configure variables for gobj package.
extern EXPCL_PANDA const int max_texture_dimension;

enum BamTextureMode {
  BTM_fullpath,
  BTM_relative,
  BTM_basename
};
extern EXPCL_PANDA BamTextureMode bam_texture_mode;

#endif


